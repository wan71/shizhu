#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"
#include "my_user.h"
#include "co_printf.h"
#include "user_task.h"
#include "button.h"
#include "driver_uart.h"
#include "driver_pmu.h"
#include "os_timer.h"
#include "driver_gpio.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "inttypes.h"
#include "driver_flash.h"
#include "os_mem.h"
#include "simple_gatt_service.h"
#include  "driver_timer.h"
#include "lcd.h"
#include "lcd_init.h"
#include "pic.h"
#include "sys_utils.h"
#include "math.h"
#define at_test 1 //蓝牙命令
#define at_imei 2
#define at_check 3
#define at_read 4
#define at_ack 5
#define at_time 6

uint16_t user_task_id;

uint8_t uint8_imei[15];//存imei
uint16_t nb_count;
uint16_t nb_num;
os_timer_t uart_at_timer;
os_timer_t oled_timer;
os_timer_t flash_read_timer;
//os_timer_t flash_start;
//os_timer_t test_timer;//模拟开始跳绳 存flash数据
uint8_t ble_at = 0;// 向串口发送NB指令类型符号
char timerev[100]=""; //开机存
char timesend[12]="";//发时间戳 蓝牙
uint32_t timeresult;//NB时间戳
int NB_state=0;
char TX_check[100]="";
char TX_read[800]="";
char TX_ack[100]="";
int NB_work_flag=0;//NB开机后要一定时间后才能用
uint32_t dt =0;//步长
uint8_t at_flash_read=0;

float last_cdt=0;
uint32_t timestart32= 0; //10位

int NB_state_ok=0;
int j =0;
uint16_t last_count=0;	
JumpData jump;
int ble_pack=0;
int cn = 1;
int i=0;
char *AT_uart_msg;
uint32_t timeble32=0;
char char_array[20]; 
float t=0;
int a;
extern float last_nb_time;
int b;
uint32_t jump_num_verify=1;
int year, month, day, hour, minute, second;

// 定义全局变量，用于保存写入成功的状态
static uint8_t write_success = 0;

//霍尔编码器用来校验的数值
// 全局变量，用于保存中断值的数组
uint32_t sum_of_last_4_interrupts;
int interrupt_counter = 0; // 用于跟踪数组中的索引位置


__attribute__((weak)) __attribute__((section("ram_code"))) void timer1_isr_ram(void)
{
    timer_clear_interrupt(TIMER1);

}

__attribute__((weak)) __attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    timer_clear_interrupt(TIMER0);
		t+=0.1;
	  a++;
		b++;
	
	if(b==10)
	{
		dt++;
		b=0;
		wdt_feed();
		
	}


	
	 if(a>=5)
	 {
		LCD_ShowIntNum(48,40,jump.count,3,RED,WHITE,16);
	  LCD_ShowIntNum(48,60,dt,4,RED,WHITE,16);
		 a=0;
	 }
   if(dt >655535)
	 {
		
		 dt=0;
	 }
	 if(last_nb_time>0)
	 {
	
		if((dt-last_nb_time)>60)
		{
			gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_7,0);
			last_nb_time=0;
		}
	 }
}





struct read_msg{

	  uint8_t pc5;
};



void TIMERX_Init(void)
{
	//定时器周期：100ms
	if(true !=timer_init(TIMER0,100000,TIMER_PERIODIC)){
		co_printf("Timer0 init failure\r\n");
	}else{
		co_printf("Timer0 init success\r\n");
	}
		if(true !=timer_init(TIMER1,100000,TIMER_PERIODIC)){
		co_printf("Timer1 init failure\r\n");
	}
		else{co_printf("Timer0 init success\r\n");
	}
		
	timer_run(TIMER0);
	timer_run(TIMER1);
	NVIC_SetPriority(TIMER0_IRQn,2);
	NVIC_EnableIRQ(TIMER0_IRQn);
	
	NVIC_SetPriority(TIMER1_IRQn,1);
	NVIC_EnableIRQ(TIMER1_IRQn);
	//timer_run(TIMER1);
	
}

struct flash_read_task 
{
uint16_t flash_read_task_start;

};

void flash_read_start(void *param)
{
	struct flash_read_task FLASH_READ_task;
	os_event_t flash_read;
	FLASH_READ_task.flash_read_task_start=1;
	flash_read.event_id=FLASH_READ;
	flash_read.param=&FLASH_READ_task;
	flash_read.param_len=sizeof(FLASH_READ_task);
	os_msg_post(user_task_id, &flash_read);

}


struct flash_save_task 
{
	uint16_t flash_save_task_start;
};


void flash_save(void *param)
{
	struct flash_save_task FLASH_SAVE_task;
	os_event_t flash_save;
	FLASH_SAVE_task.flash_save_task_start=1;
	flash_save.event_id=FLASH_SAVE;
	flash_save.param = &FLASH_SAVE_task;
  flash_save.param_len = sizeof(FLASH_SAVE_task);
	os_msg_post(user_task_id, &flash_save);
	
}

struct uart_msg{
	
	const char *wbuf;


};
//at任务
void uart_at(void *param)
{
    struct uart_msg UART_msg;
    os_event_t uart_event;
	  UART_msg.wbuf="AT+";
    uart_event.event_id = UART_AT;
    uart_event.param = (void *)&UART_msg;
    uart_event.param_len = sizeof(UART_msg);
    os_msg_post(user_task_id, &uart_event);
    
}
struct huoer_task 
{
	uint16_t huoer;
};

//1-4: 霍尔传感器的中断（下半段）
void huoer_inte(void *param)
{
	struct huoer_task HUOER_task;
	os_event_t huoer_event;
	HUOER_task.huoer=1;
	huoer_event.event_id=HUOER_INTE;
	huoer_event.param = &HUOER_task;
  huoer_event.param_len = sizeof(HUOER_task);
	os_msg_post(user_task_id, &huoer_event);
}


//霍尔编码器校验函数（上半段）   12-30目前没使用
void element_verify(uint32_t interupt)
{
	jump_num_verify++;
	huoer_inte(NULL);
	
}



//任务处理函数
//触发不同任务后 会进到这个函数 通过判断任务的id来执行对应内容
static int user_task_func(os_event_t *param)
{
	 /*
	// 读取传递的参数 
    struct NCOV_DATA *event;
	  event = param->param;
		*/	
    switch(param->event_id)
    {
			case FLASH_READ:
			{
				
				if(nb_count!=0)
				{
					int num;
					
					int i=0;
					for(i=0;i<nb_count;i++)
					{   
						num=flash_jump+(LANYA_FLASH*i);
						flash_change_nb(num);
	
			//	co_delay_100us(100);
					}
			 }
			 nb_count=0;
			 flash_nb_write(nb_count);
			 flash_erase(flash_jump,0x20000);
			 co_printf("flash_jump has been cleaned in nb\r\n");
			 
				
			}
			break;
			
			
			case FLASH_SAVE:
			{ //flash_nb_init();
				gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_7,1);
				nb_count=flash_nb_read();
				
//				const char *Message = "AT+CPSMS=2,,,\"01000011\",\"01000011\"\r\n";
//				AT_send(Message);
//			
				if(jump.count==0)
				{
					co_printf("no data \r\n");
				}
				else
				{ 
					if(NB_state==1)
					{
							int last_nb_timer=dt;
							int dt_nb;
							jump.packageindex=1;
						  jump.package=1+jump.count/package_num;
						  co_printf("jump.package is %d\r\n",jump.package);
								for(i=0;i < jump.package;i++)
								{  
									char *formattedString;
									int a;
									//co_printf("i is %d,c is %d\r\n",i,nb_count);		
					
									if(i==jump.package-1)
									{
										a=jump.count-(i*package_num);
									}
									else{
										a=package_num;
									 }
									formattedString = formatJumpData(&jump,a,i);
										// 使用 formattedString，可以根据实际需求进行处理
									//printf("%s\n", formattedString);
									jump.packageindex++;
									uart_write(UART0,(uint8_t *)formattedString, strlen((char*)formattedString));	
									printf("%s\n", formattedString);
									NB_state_ok=0;
										while(!NB_state_ok)
										{
											dt_nb=dt-last_nb_timer;
											if(dt_nb>5)
											{
												NB_state=0;
												co_printf("NB fail to it\r\n");
												break;
											}
											co_delay_100us(10000);
											
											co_printf("NB_state_ok is %d,dt_nb is %d\r\n",NB_state_ok,dt_nb);
										}
											// 不要忘记释放分配的内存
									os_free(formattedString); 
									}	
						
						}
					if(NB_state==0)
							{
								
							//char *test[512];
							int num=flash_jump+(nb_count * LANYA_FLASH);
						  
							 kanya_save(num);
							  //writer_verify_flash_datas(num,JUMP_MALLOC,(uint8_t *)formattedString);
								//flash_write(num,JUMP_MALLOC,(uint8_t *)formattedString);
								
								
							nb_count++;
							flash_nb_write(nb_count);
							co_printf("nb_count is %d\r\n",nb_count);	
						}

				co_delay_100us(100);	
				AT_send("AT+CCLK?\r\n");
				co_delay_100us(10000);
				}
				
		    jump_init(&jump);

				//flash_read_start(NULL);
			}
			break;
			
				
				case HUOER_INTE:
				{
						if(jump_num_verify%2)
						{
						
						co_printf("j=%d\r\n",j);
						//真实的通过霍尔传感器来计数 用的时候解开注释 并关掉下面test定时器 那个test是模拟测试用的				 
						jump.count++;	
            					
						jump.store[j].cindex=jump.count;
						if(jump.count==1){
						jump.store[j].jump_dt=(uint8_t)round(t * 10.0);	
							}
						else
						{
							jump.store[j].jump_dt=(uint8_t)round((t-last_cdt) * 10.0);
						}
						last_cdt=t;
						j++;	
						}
						
				
						
			}
			break;
								
				case UART_AT://at任务 蓝牙发对应的at给mcu 接着mcu通过串口发给NB得到的结果可在串口和蓝牙软件读到
				{
					
//					struct uart_msg *uart_AT_msg;
//					uart_AT_msg = (struct uart_msg *)param->param;

				
				  if(NB_work_flag==1 || NB_work_flag==0)//这个只是一个标志位，如果把flag==0的判断注释掉，就只有在NB启动后才可以用蓝牙命令了
					{
						if(ble_at==at_imei)//读imei任务 有时候NB需要输入一些AT指令可以将像下面的那些注释代码一样 用蓝牙触发这个任务来发AT 这样就不用单独改NB了
								{
//									    uint8_t TX_imei1[100]={0};
//											co_sprintf((char *)TX_imei1, "AT\r\n");
//											uart_write(UART0,(uint8_t *)TX_imei1, strlen((char*)TX_imei1));	
//											co_delay_100us(5000);  
//											uint8_t TX_imei[100]={0};

//					            co_sprintf((char *)TX_imei, "AT+LSERV=221.229.214.202,5683\r\n");//注册连平台的时候要用 只需要用一次
//											uart_write(UART0,(uint8_t *)TX_imei, strlen((char*)TX_imei));		
//											co_delay_100us(5000);  
//											uint8_t TX_imei2[100]={0};
//											co_sprintf((char *)TX_imei2, "AT+LCTM2MINIT=862295048015914\r\n");//imei号要对应好
//											uart_write(UART0,(uint8_t *)TX_imei2, strlen((char*)TX_imei2));		
//											co_delay_100us(5000);  
//									    uint8_t TX_imei3[100]={0};
//                      co_sprintf((char *)TX_imei3, "AT+REBOOT\r\n");
//				             	uart_write(UART0,(uint8_t *)TX_imei3, strlen((char*)TX_imei3));			
//                      co_delay_100us(5000);  				
											
						          

											const char *myMessage = "AT+CGSN=1\r\n";
											AT_send(myMessage);
											
						          ble_at=0;
						
			    	  	}
             else if(ble_at==at_check)
						 {
                      ble_at=0;
											char hex_str[32];
											co_sprintf(hex_str,"%x",jump.count);
											int jc;
											sscanf(hex_str, "%x", &jc);
											co_sprintf(TX_check,"AAEE0001A0%04X%02X",jump.count,0x39+jc);
											co_printf("%s\r\n",TX_check);
						 }		
            else if(ble_at==at_ack)
						{
							ble_at=0;
							co_printf("this is a upload!\r\n");
						//flash_read_start(NULL);
						
							       

						}							
            else if(ble_at==at_read)
						{ 
							ble_at=0;
							kanya_save(lanya_a1_save);
							
						
					}
								     
				
						
            else if(ble_at==at_time)
						{
							      ble_at=0;
//							    co_printf("time set\r\n");
							      co_printf("%x\r\n",timestart32);
							      co_sprintf(timesend,"%x",timestart32);
                    co_printf("%s\r\n",timesend);
							      
						}							
					}
						
								
									
				}
				break;	
				
			default:
					break;
    }
	//co_printf("#####################2");
    return EVT_CONSUMED;
}



void oled_init(void)
{
	 	 LCD_ShowChinese(0,0,"十竹科技",RED,WHITE,16,0);
	   LCD_ShowString(0,20,"LCD_W:",RED,WHITE,16,0);
		 LCD_ShowIntNum(48,20,LCD_W,3,RED,WHITE,16);
		 LCD_ShowString(0,40,"jump:",RED,WHITE,16,0);
		 LCD_ShowIntNum(48,40,jump.count,3,RED,WHITE,16);
	   LCD_ShowChinese(0,60,"时间",RED,WHITE,16,0);
		 LCD_ShowIntNum(48,60,dt,4,RED,WHITE,16);
		 LCD_ShowPicture(0,80,40,40,gImage_1);
	   //os_timer_start(&oled_timer, 500, true);
	
}

int kanya_save(int save_palce)
{							
	           uint8_t  *myArray;
							uint16_t one_pack=at_package_num;
							int AT_cout=(3*jump.count+15);					
					// 分配包含 10 个 uint32_t 元素的内存块
							myArray = (uint8_t *)os_malloc( AT_cout * sizeof(uint8_t));						
							if (myArray != NULL) {		
							for (int i = 0; i < jump.count; ++i) {					 
								myArray[i * 3+15] = (uint8_t)(jump.store[i].jump_dt);  // 将 jump_dt 放入一个字节
								myArray[i * 3 + 16] = (uint8_t)(jump.store[i].cindex >> 8);  // 将 cindex 的高字节放入一个字节
								myArray[i * 3 + 17] = (uint8_t)(jump.store[i].cindex);  // 将 cindex 的低字节放入一个字节

//								myArray[i * 3] = (uint8_t)(jump.store[i].jump_dt);  // 将 jump_dt 放入一个字节
//								myArray[i * 3 + 1] = (uint8_t)(jump.store[i].cindex >> 8);  // 将 cindex 的高字节放入一个字节
//								myArray[i * 3 + 2] = (uint8_t)(jump.store[i].cindex);  // 将 cindex 的低字节放入一个字节
									
									// 在这里可以进行其他处理，例如打印数据
							
							
							printf("Index %d: jump_dt=%u, cindex=%u\r\n", i, myArray[i*3+15], myArray[i*3 + 16] << 8 | myArray[i*3 + 17]);
							}
							  
								myArray[0]=(uint8_t)(AT_cout>>8);
								myArray[1]=(uint8_t)(AT_cout);
								myArray[2]=(uint8_t)(jump.mode>>8);
								myArray[3]=(uint8_t)(jump.mode);
								myArray[4]= (uint8_t)(jump.count >> 8); 
								myArray[5]= (uint8_t)(jump.count); 
								myArray[6]= (uint8_t)(jump.jump_time >> 24); 
								myArray[7]= (uint8_t)(jump.jump_time >> 16); 
								myArray[8]= (uint8_t)(jump.jump_time >> 8); 
								myArray[9]= (uint8_t)(jump.jump_time); 
							  myArray[10]=myArray[11]=myArray[12]=myArray[13]=myArray[14]=0;
							  co_printf("one_pack is %d,jump.count is %d,jump_time is %d\r\n",myArray[0] << 8 | myArray[1] ,myArray[4] << 8 | myArray[5], myArray[6] << 24 | myArray[7]<<16|myArray[8] << 8 | myArray[9]);
							
								co_delay_100us(10);
							// flash_erase(save_palce,0x1000);
							//writer_verify_flash_datas(save_palce,AT_cout,(uint8_t *)myArray);
							flash_write(save_palce,AT_cout,(uint8_t *)myArray);
						    os_free(myArray);		
								return AT_cout;	
						}else{
							os_free(myArray);
							co_printf("myArray  fail to build!!!\r\n");
							return 0;
						}

}	
		





//任务初始化函数
//因为硬件还没完全可以 所以测试功能时多用定时器来模拟跳绳
void user_task_init(void)
{	
    user_task_id = os_task_create(user_task_func);
	
    
		TIMERX_Init();//自己加的，单独控制oled的更新  void timer0_isr_ram(void)
		

}


uint16_t TX_NUM(indexdata a[]){
	int n =0;

	while(n<package_num){
		if(a[n].cindex==0){
			break;
		}
		n++;
	}
	return n;
	
}


void at_uart_init(char *AT_uart_msg)
{
		//co_printf("AT_uart_msg is %s\r\n\r\n",AT_uart_msg);	
		 if(strncmp("+CGSN:",AT_uart_msg,5)==0)//imei
				{
					char imeitmp[20]=""; //存imei
					
					uint32_t flash_length;
					size_t imeiSize;
					
					strmncpy(AT_uart_msg, 8, 15, imeitmp);
					
					
					imeiSize = strlen(imeitmp);
					flash_length = imeiSize * sizeof(uint8_t);
					
					co_printf("imeiSize is %d\r\n",flash_length);
					
					str_to_uint8_t_array(imeitmp, uint8_imei, imeiSize);
					
					writer_verify_flash_datas(flash_imei, flash_length, (uint8_t *)uint8_imei);
			
					flash_read(flash_imei, flash_length, (uint8_t *)uint8_imei);
					// Print the result
						for (size_t i = 0; i < imeiSize; i++) {
								co_printf("%d", uint8_imei[i]);
						}
						co_printf("\r\n");
						
					}	
			
				else if(strncmp("OK",AT_uart_msg,2)==0)	
						{
							NB_state_ok=1;
							//co_printf("this is already send!\r\n");


						}
				else if(strncmp("+LWM2MEVENT: 2",AT_uart_msg,14)==0) //LWM2MEVENT显示的是状态，为2是成功
						{
							if(at_flash_read==0)
							{
							flash_read_start(NULL);
							co_printf("NB subscription succeeded\r\n");
							}
							else{
							co_printf("lanya add is working\r\n");
							}
							NB_state=1;

						}	
			else if(strncmp("+LWM2MEVENT: 1",AT_uart_msg,14)==0) //LWM2MEVENT显示的是状态，为2是成功
						{
							
							NB_state=0;
							

						}	
				else if(strncmp("+CTZE:",AT_uart_msg,5)==0)//时间戳 
				{
						strmncpy(AT_uart_msg, 12, 19, timerev);
						//strmncpy(AT_uart_msg, 6, 13, timerev);
						//co_printf("timerev:%s\r\n",timerev);
						convertToSixInt(timerev, &year, &month, &day, &hour, &minute, &second);
						
						//timestart = getTimestamp(year, month, day, hour, minute, second);//发NB的    这个时间是64位的
						
						timestart32 = getTimestamp32(year, month, day, hour, minute, second);//发蓝牙的  这个时间是32位的
						//co_printf("%d\r\n", timestart32);
						uint16_t high_bits = (uint16_t)(timestart32 >> 16);
						uint16_t low_bits = (uint16_t)(timestart32 & 0xFFFF);
						low_bits = low_bits-dt;
						jump.jump_time = ((uint32_t)high_bits << 16) | low_bits;
		
					co_printf("jump.jump_time is %d\r\n",jump.jump_time);

					}					
				
				  else	if(strncmp("+CCLK:",AT_uart_msg,5)==0)//时间戳
					{
						//strmncpy(AT_uart_msg, 12, 19, timerev);
						strmncpy(AT_uart_msg, 7, 19, timerev);
						//co_printf("timerev:%s\r\n",timerev);
						convertToSixInt(timerev, &year, &month, &day, &hour, &minute, &second);
						
						//timestart = getTimestamp(year, month, day, hour, minute, second);//发NB的    这个时间是64位的
						
						timestart32 = getTimestamp32(year, month, day, hour, minute, second);//发蓝牙的  这个时间是32位的
						//co_printf("%d\r\n", timestart32);
						uint16_t high_bits = (uint16_t)(timestart32 >> 16);
						uint16_t low_bits = (uint16_t)(timestart32 & 0xFFFF);
						low_bits = low_bits-dt;
						jump.jump_time = ((uint32_t)high_bits << 16) | low_bits;
		
					co_printf("new jump_time is %d\r\n",jump.jump_time);

					}							
										
}


