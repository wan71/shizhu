#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"

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

#include "lcd.h"
#include "lcd_init.h"
#include "pic.h"
#include "sys_utils.h"

#define at_test 1 //蓝牙命令
#define at_imei 2
#define at_check 3
#define at_read 4
#define at_ack 5
#define at_time 6

uint16_t user_task_id;

os_timer_t uart_at_timer;

os_timer_t read_timer;

os_timer_t test_timer;//模拟开始跳绳 存flash数据

uint8_t ble_at = 0;// 向串口发送NB指令类型符号

char timerev[100]=""; //开机存时间用

char imeirev[100]=""; //存imei

char timesend[12]="";//发时间戳 蓝牙

char timeresult[21]="";//NB时间戳

char TX_check[100]="";

char TX_read[100]="";

char TX_ack[100]="";

int NB_work_flag=0;//NB开机后要一定时间后才能用

uint64_t dt =0;//步长

uint32_t cdt =0;//步长

uint64_t timestart= 0; //11位

uint32_t timestart32= 0; //10位

uint64_t mstime = 0; 

int jump_flag=0;

uint16_t wait =0;

int test_count=0;

int j =0;

uint16_t last_count=0;	

uint8_t TX[700]={0};

JumpData jump ;

int ble_pack=0;

int cn = 1;

char ble_time[20]="";

uint32_t timeble32=0;

float t=0;

uint8_t PC6_V=0;

uint8_t PC7_V=0;

uint8_t jumpflag=0;
uint8_t jumpflag1=0;
uint8_t jumpflag2=0;

uint16_t jump_num=0;

//JumpData jstore[30];

//long int timetest = 0;

int year, month, day, hour, minute, second;

//时间定时器 因为跳绳时需要时间数据 而想得到时间的方法有三 1.NB开机时会吐出时间 2.蓝牙设置 3.用AT指令通过NB询问时间
//为了方便 我在NB开机吐出时间的时候将其保存了起来 并用一个定时器不断改变这个时间 当作实时时间 不过可能误差较大
//蓝牙设置 可看蓝牙文档 我也已经写好可以用了
static void uart_timeout_handler(void *param)//时间戳递增
{
//    uart_at(NULL);
	    dt++;
	    cdt++;
	    if(dt%10==0)
			{
//				timestart++;
//				timestart32++;
//				co_printf("%s\r\n",timestart);
//			  dt=0;			
//				co_printf("%d\r\n", cdt);
			}
			if(dt>60000)
				{
				dt=0;
			}
				
			if(cdt>60000)
				{
				cdt=0;
			}
				
			mstime = timestart*10;
			
			addElevenDigitNumbers(mstime, dt, timeresult);
			
	    co_sprintf(timesend,"%x",timestart32);//蓝牙的
			
			
			

}

struct read_msg{

	  uint8_t pc5;
};

//显示屏 按钮定时监听 用霍尔传感器计跳绳数 这是真是模拟 以后跳绳也是要靠霍尔这样来计数的 不用的话就把735的定时器启动函数注释掉
static void read_handler(void *param)//时间戳递增
{
    uart_Read(NULL);
		 LCD_ShowChinese(0,0,"十竹科技",RED,WHITE,16,0);
	   LCD_ShowString(0,20,"LCD_W:",RED,WHITE,16,0);
		 LCD_ShowIntNum(48,20,LCD_W,3,RED,WHITE,16);
		 LCD_ShowString(0,40,"jump:",RED,WHITE,16,0);
		 LCD_ShowIntNum(48,40,jump_num,3,RED,WHITE,16);
	   LCD_ShowChinese(0,60,"时间",RED,WHITE,16,0);
		 LCD_ShowFloatNum1(48,60,t,4,RED,WHITE,16);
	   t+=0.1;
	   LCD_ShowPicture(0,80,40,40,gImage_1);
	
	   if(PC6_V != gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_6))//霍尔传感器计数 pc6 pc7电平变化两次就当跳了一次 显然不准 需要用算法优化一下
			 {
	       co_printf("PC6:%d\r\n",gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_6));

				  jumpflag++;
		   }
			 else if(PC7_V != gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_7))
			 {
				 co_printf("PC7:%d\r\n",gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_7));
				 PC7_V=gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_7);

				   jumpflag++;
			 }

			 if(jumpflag==2){//变化两次 按理应该是轮流变化一次才算跳了一圈 但因为霍尔的灵敏度等问题 可能跳了一圈 pc6变了两次 而pc7没变 需要优化
				 jumpflag=0;
				 jump_num++;
				
//真实的通过霍尔传感器来计数 用的时候解开注释 并关掉下面test定时器 那个test是模拟测试用的				 
		jump.count++;											
		jump.store[j].cindex=jump.count;
		jump.store[j].jump_dt=cdt;											
	  j++;	
	 if(j==20){
		 
		uint8_t TX_head[250]={0};
	  uint8_t TX_end[450]={0};
		
		int a=TX_NUM(jump.store);
										
		generateString(a,jump.store,TX_end);
		
    co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
		
		concatenateStrings(TX_head,TX_end,TX);		 
											
  	flash_write(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
		 
		jump.package++;
	  jump.packageindex++;
											
		co_printf("%d\r\n",jump.count);
		j=0;
		
		for (int k = 0; k < 20; k++) {
       jump.store[k].jump_dt = 0;
       jump.store[k].cindex = 0;
     }
	 }				 
//这里到上一条注释之间都是霍尔传感器真实计数代码				 
			 }			 
		 
		 
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
//显示按钮监听任务
void uart_Read(void *param)
{
	  struct read_msg READ_msg;
    os_event_t read_event;
	  READ_msg.pc5=gpio_get_pin_value(GPIO_PORT_C, GPIO_BIT_5);
	 
	

    read_event.event_id = UART_READ;
    read_event.param = (void *)&READ_msg;
    read_event.param_len = sizeof(READ_msg);
    os_msg_post(user_task_id, &read_event);

}
//一些字符串处理函数 用于处理NB返回的信息 从中提取所需信息
void strmncpy(char *s, int m, int n, char *t)
{
    int k=0;
    while(s[k]!=0)
    {
    	k++;
    }
    if(k<=m)
    {
        *t='\0';
        return;
    }
    char *p=s+m;
    for(int i=0;i<n;i++)
    {
        *t=*p;
        if(*t=='\0')
        {
   	    return;
        } 
        t++;
        p++;
    }
    *t='\0';
}

//任务处理函数
//触发不同任务后 会进到这个函数 通过判断任务的id来执行对应内容
static int user_task_func(os_event_t *param)
{

									
    switch(param->event_id)
    {
			  case UART_READ://按下按钮 拉高EN使能NB 
				    {
								struct read_msg *uart_READ_msg;
					      uart_READ_msg = (struct read_msg *)param->param;
							  if(uart_READ_msg->pc5==0)
								{

//	                  system_set_port_pull(GPIO_PD1, true);
//									  system_set_port_mux(GPIO_PORT_D, GPIO_BIT_1, PORTD1_FUNC_D1);

									 gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_7,1);
//									      co_printf("PB7:%d\r\n",gpio_get_pin_value(GPIO_PORT_B, GPIO_BIT_7));
									 j=0;
									 jump.count=0;
									 jump_num=0;
									 jump.package=1;
									 jump.packageindex=1;
									  flash_erase(0x70000,0x2F000);
									  co_printf("change\r\n");
								}
							
//							  co_printf("PC5:%d\r\n",uart_READ_msg->pc5);
								
								
						}
					  break;
			
        case USER_EVT_BUTTON://这个是sdk自带的 没有删除 也没用到
            {
                struct button_msg_t *button_msg;
                const char *button_type_str[] = {
                                                    "BUTTON_PRESSED",
                                                    "BUTTON_RELEASED",
                                                    "BUTTON_SHORT_PRESSED",
                                                    "BUTTON_MULTI_PRESSED",
                                                    "BUTTON_LONG_PRESSED",
                                                    "BUTTON_LONG_PRESSING",
                                                    "BUTTON_LONG_RELEASED",
                                                    "BUTTON_LONG_LONG_PRESSED",
                                                    "BUTTON_LONG_LONG_RELEASED",
                                                    "BUTTON_COMB_PRESSED",
                                                    "BUTTON_COMB_RELEASED",
                                                    "BUTTON_COMB_SHORT_PRESSED",
                                                    "BUTTON_COMB_LONG_PRESSED",
                                                    "BUTTON_COMB_LONG_PRESSING",
                                                    "BUTTON_COMB_LONG_RELEASED",
                                                    "BUTTON_COMB_LONG_LONG_PRESSED",
                                                    "BUTTON_COMB_LONG_LONG_RELEASED",
                                                };

                button_msg = (struct button_msg_t *)param->param;
                
                co_printf("KEY 0x%08x, TYPE %s.\r\n", button_msg->button_index, button_type_str[button_msg->button_type]);
            }
            break;
						
				case UART_AT://at任务 蓝牙发对应的at给mcu 接着mcu通过串口发给NB得到的结果可在串口和蓝牙软件读到
				{
					
//					struct uart_msg *uart_AT_msg;
//					uart_AT_msg = (struct uart_msg *)param->param;

				
				  if(NB_work_flag==1 || NB_work_flag==0)//这个只是一个标志位，如果把flag==0的判断注释掉，就只有在NB启动后才可以用蓝牙命令了
					{
				    	if(ble_at==at_test)//这个只是测试时候用的一个任务 用蓝牙来触发任务 串口打印flash里面的数据 后面好像没用了
								{
									
//									printf("%d",os_get_free_heap_size());
											

//									int j=0;
//									while(j<20)
//		              	{
//											jump.count++;
//											
//		 		              jump.store[j].cindex=jump.count;
//											jump.store[j].jump_dt=cdt;
////											jump.store[i+1].cindex=0;
////											jump.store[i+1].jump_dt=0;
//											
//											j++;
//		              	}
										
//										int a=TX_NUM(jump.store);
//										
//										generateString(a,jump.store,TX_end);
										
//										co_printf("%s",TX_end);
			             
									    
//					    		    co_sprintf((char *)TX, "AT+LCTM2MSEND=1,\"{\"serviceId\": 2, \"rsrp\":%d, \"sinr\":%d, \"pci\":0, \"ecl\":0, \"cell_id\":0}\"\r\n",uart_AT_msg->pc6,uart_AT_msg->pc7 );
//   				            co_sprintf((char *)TX, "AT+CGMI\r\n" );
//				              co_sprintf((char *)TX, "AT+LCTM2MSEND=1,\"{%d, \"sinr\":%d}\"\r\n",uart_AT_msg->pc6,uart_AT_msg->pc7 );
						      
//							    		co_sprintf((char *)TX, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"{index:%d,dt:%d;index:%d,dt:%d;index:%d,dt:%d;}\"}\"\r\n",
//											jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
																			
//									    co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
//											jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
						

//											concatenateStrings(TX_head,TX_end,TX);
											
											
//											flash_erase(0x7D000,0x1000);
//											
//											flash_write(0x7D000,sizeof(TX),TX);
//                      for(int cn=0;cn<jump.package-1;cn++){

//													flash_read(0x70000+2*sizeof(TX),sizeof(TX),TX);
//													printf("%s",TX);
//													printf("%d",jump.package);
												 flash_read(0x70000+(jump.package-2)*sizeof(TX),sizeof(TX),TX);
//												 	flash_read(0x7D000+cn*sizeof(TX),sizeof(TX),TX);
				
				                  uart_write(UART0,(uint8_t *)TX, strlen((char*)TX));	
												  printf("%s",TX);

												
//											}
											if(jump.package==1){
		                      uint8_t TX_head[250]={0};
	                        uint8_t TX_end[450]={0};
		 
		                      int a=TX_NUM(jump.store);
										
	                      	generateString(a,jump.store,TX_end);
		
                          co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		                      jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
		
		                      concatenateStrings(TX_head,TX_end,TX);		 												
													uart_write(UART0,(uint8_t *)TX, strlen((char*)TX));	
												  printf("%s",TX);
											}                    									
											
											
				      	}
				     	else if(ble_at==at_imei)//读imei任务 有时候NB需要输入一些AT指令可以将像下面的那些注释代码一样 用蓝牙触发这个任务来发AT 这样就不用单独改NB了
								{
									    uint8_t TX_imei1[100]={0};
											co_sprintf((char *)TX_imei1, "AT\r\n");
											uart_write(UART0,(uint8_t *)TX_imei1, strlen((char*)TX_imei1));	
											co_delay_100us(5000);  
											uint8_t TX_imei[100]={0};

					            co_sprintf((char *)TX_imei, "AT+LSERV=221.229.214.202,5683\r\n");//注册连平台的时候要用 只需要用一次
											uart_write(UART0,(uint8_t *)TX_imei, strlen((char*)TX_imei));		
											co_delay_100us(5000);  
											uint8_t TX_imei2[100]={0};
											co_sprintf((char *)TX_imei2, "AT+LCTM2MINIT=862295048015914,1\r\n");//imei号要对应好
											uart_write(UART0,(uint8_t *)TX_imei2, strlen((char*)TX_imei2));		
											co_delay_100us(5000);  
									    uint8_t TX_imei3[100]={0};
                      co_sprintf((char *)TX_imei3, "AT+REBOOT\r\n");
				             	uart_write(UART0,(uint8_t *)TX_imei3, strlen((char*)TX_imei3));			
                      co_delay_100us(5000);  				
											
						          
											//uint8_t TX_imei[100]={0};
							        co_sprintf((char *)TX_imei, "AT+CGSN=1\r\n");
											uart_write(UART0,(uint8_t *)TX_imei, strlen((char*)TX_imei));		
											co_printf( "AT+CGSN=1\r\n");
						          ble_at=0;
						
			    	  	}
             else if(ble_at==at_check)
						 {
                      ble_at=0;
											char hex_str[32];
											co_sprintf(hex_str,"%x",jump.count);
//							        co_printf("%x,",jump.count);
											int jc;
											sscanf(hex_str, "%x", &jc);
//							        co_printf("%x,",jc);
//							        co_printf("%x,",0x39+jc);
											co_sprintf(TX_check,"AAEE0001A0%04X%02X",jump.count,0x39+jc);
											co_printf("%s\r\n",TX_check);
						 }		
            else if(ble_at==at_ack)
						{
										 ble_at=0;

							       if(cn>=ble_pack){
											 co_sprintf(TX_ack,"finish\r\n");	
											 co_printf("%s\r\n",TX_ack);
											 cn=0;
											 jump.package=1;
											 jump.packageindex=1;
											 jump.count=0;
											 j=0;
											 jump_num=0;
											 flash_erase(0x70000,0x2F000);
										 }
										 else{
							          co_printf("ACK\r\n");
							          flash_read(0x70000+(cn)*sizeof(TX),sizeof(TX),TX);			
                        co_printf("%s\r\n",TX);
											  uart_write(UART0,(uint8_t *)TX, strlen((char*)TX));		
							          char result[100];
                        extract_index_dt((char *)TX, result);							 
                        co_sprintf(TX_ack,"AAFF%04x%04x%xA1%04x%s\r\n",ble_pack,jump.count,timestart32,cn+1,result);							
//							        co_sprintf(TX_ack,"AAFF0002A10001%s",extractIndexAndDt((char *)TX));
										    co_printf("%s\r\n",TX_ack);
											 cn++;
										 }
							       

						}							
            else if(ble_at==at_read)
						{
                     ble_at=0;
							       co_printf("Blue_teeth read\r\n");
							       ble_pack=jump.count/20;
							       if((jump.count%20)>0) {
											 ble_pack++; //当跳绳次数不是包数的倍数时最后一个包会不足20个 需要想办法打包保存发送
											 
											 		uint8_t TX_head[250]={0};
	                        uint8_t TX_end[450]={0};
		
		                      int a=TX_NUM(jump.store);
										
		                      generateString(a,jump.store,TX_end);
		
                          co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		                      jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
		
		                      concatenateStrings(TX_head,TX_end,TX);		 
											
  	                      flash_write(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
		 
		                      jump.package++;
	                        jump.packageindex++;
											
		                      co_printf("%d\r\n",jump.count);
		                      j=0;
										 }
							       if(cn==0)cn=1;
							   
							        cn--;
							        
//							       if(cn==0){
//											 co_sprintf(TX_read,"AAFF%04xA1%04x%04X%04x%04X%4x",cn+1,cn,ble_pack,jump.mode,jump.count,timestart32);
//							         co_printf(TX_read);
//										 }
//										 else{
											 flash_read(0x70000+(cn)*sizeof(TX),sizeof(TX),TX);	
							         co_printf("%s\r\n",TX);
							         uart_write(UART0,(uint8_t *)TX, strlen((char*)TX));		
											 char result[100];
                       extract_index_dt((char *)TX, result);
											 co_sprintf(TX_read,"AAFF%04x%04x%xA1%04x%s\r\n",ble_pack,jump.count,timestart32,cn+1,result);	//AAFF 总包数 总次数 时间戳 A1 包序号 数据
											 co_printf("%s\r\n",TX_read);
							         cn++;
//										 }
								     
						}		 
            else if(ble_at==at_time)
						{
							      ble_at=0;
//							      co_printf("time set\r\n");
							      co_printf("%x\r\n",timestart32);
							      co_sprintf(timesend,"%x",timestart32);
                    co_printf("%s\r\n",timesend);
							      
						}							
					}
						
								
							
				}
				break;
				
				case AT_UART://这里处理 NB初始化时吐出的信息
				{
					       //这里接受NB的字符串，需要判断接受信息然后保存strncmp比较前n个字符
					      char *AT_uart_msg = (char *)param->param;
					      
					      
					      co_printf("%s",AT_uart_msg);
					     
					      
//					      co_printf("%d",strncmp("+LWM2MEVENT: 2",AT_uart_msg,13));
					      
					
					
					      if(strncmp("+CTZE:",AT_uart_msg,5)==0)//时间戳
								{
									strmncpy(AT_uart_msg, 12, 19, timerev);
									co_printf("%s\r\n",timerev);
									convertToSixInt(timerev, &year, &month, &day, &hour, &minute, &second);
									
									timestart = getTimestamp(year, month, day, hour, minute, second);//发NB的
									
									timestart32 = getTimestamp32(year, month, day, hour, minute, second);//发蓝牙的
									
									os_timer_start(&uart_at_timer, 100, true);
									
//									os_timer_start(&printf_timer, 1000, true);

									co_printf("%x\r\n", timestart32);
									
									co_printf("%d\r\n", timestart32);
									
									mstime = timestart*10;
                  addElevenDigitNumbers(mstime, dt, timeresult);
									co_printf("%s\r\n", timeresult);
//									timetest = getTimestamp(2023, 1, 12, 22, 29, 31);
//									
// 								  co_printf("%x\r\n",timetest);
									
									
								}
					      else if(strncmp("+LWM2MEVENT: 2",AT_uart_msg,14)==0)
									{
									     NB_work_flag=1;
										
											 jump.deviceId="123";

                       addElevenDigitNumbers(mstime, dt, jump.jump_time);
	                     jump.mode=1;
	                     jump.package=1;
	                     jump.packageindex=1;
	                     jump.count=0;
	
	                   	for (int k = 0; k < 20; k++) {
                         jump.store[k].jump_dt = 0;
                         jump.store[k].cindex = 0;
                        }
										flash_erase(0x70000,0x2F000);
										os_timer_start(&test_timer, 150, true);
					          co_printf("OK\r\n");

				          }				
					      else if(strncmp("+CGSN:",AT_uart_msg,5)==0)//imei
								{
									char imeitmp[100]=""; //存imei
									strmncpy(AT_uart_msg, 8, 15, imeitmp);
									co_sprintf(imeirev,"A3%s",imeitmp);
									co_printf("%s\r\n",imeirev);
								}									
								
//					      if(strcmp("abc",AT_uart_msg)==0){
//					          co_printf("yes\r\n");

//				         }
					
					
				}
				break;
    }

    return EVT_CONSUMED;
}
//测试用的中断 用定时器来定时增加跳绳数 模拟霍尔传感器 需要使用的话可以解开655行的定时器启动注释
static void test_handler(void *param)//跳绳
{
		//co_printf("test_handler start\r\n");
		jump.count++;
											
		jump.store[j].cindex=jump.count;
		jump.store[j].jump_dt=cdt;
											
	  j++;
	
	
	 if(j==20){
		 
		uint8_t TX_head[250]={0};
	  uint8_t TX_end[450]={0};
		
		int a=TX_NUM(jump.store);
										
		generateString(a,jump.store,TX_end);
		
    co_sprintf((char *)TX_head, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%s\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
		jump.deviceId,jump.package,jump.packageindex,jump.jump_time,jump.mode,jump.count);
		
		concatenateStrings(TX_head,TX_end,TX);		 

											
  	flash_write(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
//					co_printf("%s\r\n",TX);
//		flash_read(0x70000+(jump.packageindex-1)*sizeof(TX),sizeof(TX),TX);
		 
		jump.package++;
	  jump.packageindex++;
											
		co_printf("%d\r\n",jump.count);
//		
//			co_printf("%s\r\n",TX);
		
//		flash_read(0x7D000,sizeof(TX),TX);
//					co_printf("%s\r\n",TX);
		 j=0;
	 }
	 if(jump.package>4)
	 {
		 	os_timer_stop(&test_timer);
	 }

}
//任务初始化函数
//因为硬件还没完全可以 所以测试功能时多用定时器来模拟跳绳
void user_task_init(void)
{	
    user_task_id = os_task_create(user_task_func);
	
    os_timer_init(&uart_at_timer, uart_timeout_handler,NULL);	//时间戳递增
	  os_timer_init(&read_timer, read_handler,NULL);	//读取输出
	  os_timer_init(&test_timer, test_handler,NULL);	//模拟跳绳
	  os_timer_start(&read_timer, 30, true);

}

//时间数据提取
//下面函数都是提取NB开机时吐出信息所用到的字符串处理函数
void convertToSixInt(const char* datetime, int* year, int* month, int* day, int* hour, int* minute, int* second) {
    char datetime_copy[20];
    strcpy(datetime_copy, datetime);

    char* token = strtok(datetime_copy, "/ :");
    *year = atoi(token);

    token = strtok(NULL, "/ :");
    *month = atoi(token);

    token = strtok(NULL, "/ :,");
    *day = atoi(token);

    token = strtok(NULL, "/ :");
    *hour = atoi(token);

    token = strtok(NULL, "/ :");
    *minute = atoi(token);

    token = strtok(NULL, "/ :");
    *second = atoi(token);
}




int isLeapYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
        return 1; // 是闰年
    } else {
        return 0; // 不是闰年
    }
}
int daysOfMonth(int year, int month) {
    int days;
    switch (month) {
        case 2:
            days = isLeapYear(year) ? 29 : 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            days = 30;
            break;
        default:
            days = 31;
            break;
    }
    return days;
}
uint64_t getTimestamp(int year, int month, int day, int hour, int minute, int second) {
    int days = 0;
    
    // 计算年份总天数
    for (int i = 1970; i < year; i++) {
        days += isLeapYear(i) ? 366 : 365;
    }
    
    // 计算月份总天数
    for (int i = 1; i < month; i++) {
        days += daysOfMonth(year, i);
    }
    
    // 加上当前月份的天数
    days += day - 1;
    
    // 计算时间戳
    uint64_t timestamp = ((days * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second) - 8 * 60 * 60);
    
    return timestamp;
}

uint32_t getTimestamp32(int year, int month, int day, int hour, int minute, int second) {
    int days = 0;
    
    // 计算年份总天数
    for (int i = 1970; i < year; i++) {
        days += isLeapYear(i) ? 366 : 365;
    }
    
    // 计算月份总天数
    for (int i = 1; i < month; i++) {
        days += daysOfMonth(year, i);
    }
    
    // 加上当前月份的天数
    days += day - 1;
    
    // 计算时间戳
    uint32_t timestamp = ((days * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second) - 8 * 60 * 60);
    
    return timestamp;
}

void addElevenDigitNumbers(uint64_t a, uint64_t b, char* result) {
    uint64_t sum = a + b;
    sprintf(result, "%" PRIu64, sum);
}


uint16_t TX_NUM(indexdata a[]){
	int n =0;

	while(n<20){
		if(a[n].cindex==0){
			break;
		}
		n++;
	}
	return n;
	
}

void  generateString(int a,indexdata b[],uint8_t TX[]) {
//    int length = (12 + 12) * a + 5; // 每组数据占据的长度为12个字符（包括逗号、冒号和分号），每个数字占据的长度为10个字符（包括负号）
    
    char str[450] ;

    
    strcpy(str, "{");
    int offset = 1;
    for (int i = 0; i < a; i++) {
        offset += co_sprintf(str + offset, "index:%d,dt:%d;", b[i].cindex, b[i].jump_dt);
    }
    
    strcat(str, "}\"}\"\r\n");//}}"\r\n
    for (int i = 0; i < 450; i++) {
        TX[i] = (uint8_t)str[i];
		} 
    
}
void concatenateStrings(uint8_t TX1[], uint8_t TX2[], uint8_t TX[]) {
    uint8_t* p = TX;
    
    while (*TX1 != '\0') { // 复制TX1数组的内容到TX数组中
        *p++ = *TX1++;
    }
    
    while (*TX2 != '\0') { // 复制TX2数组的内容到TX数组中
        *p++ = *TX2++;
    }
    
    *p = '\0'; // 在TX数组的末尾添加字符串结束符
}


void extract_index_dt(const char* str, char* hex_result) {
    const char* start = strchr(str, '{');
    if (start == NULL) {
        return;
    }
    const char* end = strchr(start, '}');
    if (end == NULL) {
        return;
    }
    start += 1;
    const char* index_start;
    const char* dt_start;
    char* pos = hex_result;
    while (start < end) {
        index_start = strstr(start, "index:");
        if (index_start == NULL) {
            return;
        }
        index_start += 6;
        dt_start = strstr(start, "dt:");
        if (dt_start == NULL) {
            return;
        }
        dt_start += 3;
        // 提取index和dt的十六进制数值并保存在hex_result数组中
        unsigned int index = strtoul(index_start, NULL, 10);
        unsigned int dt = strtoul(dt_start, NULL, 10);
        
        // 转换为两位十六进制数并保存在hex_result数组中
        sprintf(pos, "%02X%02X", index, dt);
        pos += 4;
        start = strstr(dt_start, ";");
        if (start == NULL) {
            return;
        }
        start += 1;
    }
}

