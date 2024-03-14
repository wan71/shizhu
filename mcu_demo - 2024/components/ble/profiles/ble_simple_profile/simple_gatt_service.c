/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "driver_flash.h"
#include "my_user.h"
#include "user_task.h"
#include "simple_gatt_service.h"
#include "os_mem.h"

/*
 * MACROS (宏定义)
 */
extern uint8_t ble_at;

extern uint8_t uint8_imei;
extern uint16_t nb_count;
uint8_t ble_flag=0;//帧头判断符

uint8_t ble_id;//事件类型

extern uint32_t *myArray;

extern uint32_t timeble32;



extern JumpData jump;

extern uint32_t timestart32;

extern char TX_read[100];

extern int ble_pack;

extern char TX_ack[100];

enum ble_event { //事件类型有下面几种，在接受到蓝牙信息时确定
    TIME=1,
    CLEAN,
	  CHECK,
	  READ,
	  VISON,
	  IMEI,
	  BATT,
	  ANS,
};


extern char timesend[100]; //发时间用
extern uint8_t at_flash_read;
extern char imeirev[100]; //存imei
//extern char imeireva[100]; 
extern char TX_check[100];
int m;
/*
 * CONSTANTS (常量定义)
 */


// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t sp_svc_uuid[] = UUID16_ARR(SP_SVC_UUID);

/******************************* Characteristic 1 defination *******************************/
// Characteristic 1 UUID: 0xFFF1
// Characteristic 1 data 
#define SP_CHAR1_VALUE_LEN  200
uint8_t sp_char1_value[SP_CHAR1_VALUE_LEN] = {0};
// Characteristic 1 User Description
#define SP_CHAR1_DESC_LEN   17
const uint8_t sp_char1_desc[SP_CHAR1_DESC_LEN] = "Characteristic 1";

/******************************* Characteristic 2 defination *******************************/
// Characteristic 2 UUID: 0xFFF2
// Characteristic 2 data 
#define SP_CHAR2_VALUE_LEN  200
uint8_t sp_char2_value[SP_CHAR2_VALUE_LEN] = {0};
// Characteristic 2 User Description
#define SP_CHAR2_DESC_LEN   17
const uint8_t sp_char2_desc[SP_CHAR2_DESC_LEN] = "Characteristic 2";

/******************************* Characteristic 3 defination *******************************/
// Characteristic 3 UUID: 0xFFF3
// Characteristic 3 data 
#define SP_CHAR3_VALUE_LEN  30
uint8_t sp_char3_value[SP_CHAR3_VALUE_LEN] = {0};
// Characteristic 3 User Description
#define SP_CHAR3_DESC_LEN   17
const uint8_t sp_char3_desc[SP_CHAR3_DESC_LEN] = "Characteristic 3";

/******************************* Characteristic 4 defination *******************************/
// Characteristic 4 UUID: 0xFFF4
// Characteristic 4 data 
#define SP_CHAR4_VALUE_LEN  40
uint8_t sp_char4_value[SP_CHAR4_VALUE_LEN] = {0};
// Characteristic 4 client characteristic configuration
#define SP_CHAR4_CCC_LEN   2
uint8_t sp_char4_ccc[SP_CHAR4_CCC_LEN] = {0};
// Characteristic 4 User Description
#define SP_CHAR4_DESC_LEN   17
const uint8_t sp_char4_desc[SP_CHAR4_DESC_LEN] = "Characteristic 4";

/******************************* Characteristic 5 defination *******************************/
// Characteristic 5 UUID: 0xFFF5
uint8_t sp_char5_uuid[UUID_SIZE_2] =
{ 
  LO_UINT16(SP_CHAR5_UUID), HI_UINT16(SP_CHAR5_UUID)
};
// Characteristic 5 data 
#define SP_CHAR5_VALUE_LEN  50
uint8_t sp_char5_value[SP_CHAR5_VALUE_LEN] = {0};
// Characteristic 5 User Description
#define SP_CHAR5_DESC_LEN   17
const uint8_t sp_char5_desc[SP_CHAR5_DESC_LEN] = "Characteristic 5";

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
uint8_t sp_svc_id = 0;
uint8_t ntf_char1_enable = 0;

/*
 * LOCAL VARIABLES (本地变量)
 */
static gatt_service_t simple_profile_svc;

/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个attribute的定义。
 * 第一个attribute为Service 的的定义。
 * 每一个特征值(characteristic)的定义，都至少包含三个attribute的定义；
 * 1. 特征值声明(Characteristic Declaration)
 * 2. 特征值的值(Characteristic value)
 * 3. 特征值描述符(Characteristic description)
 * 如果有notification 或者indication 的功能，则会包含四个attribute的定义，除了前面定义的三个，还会有一个特征值客户端配置(client characteristic configuration)。
 *
 */

const gatt_attribute_t simple_profile_att_table[SP_IDX_NB] =
{
    // Simple gatt Service Declaration
    [SP_IDX_SERVICE]                        =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    UUID_SIZE_2,                                                /* Max size of the value */     /* Service UUID size in service declaration */
                                                    (uint8_t*)sp_svc_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
                                                },

        // Characteristic 1 Declaration           
        [SP_IDX_CHAR1_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 1 Value                  
        [SP_IDX_CHAR1_VALUE]                =   {
                                                    { UUID_SIZE_16, SP_CHAR1_TX_UUID },                 /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_NOTI,                           /* Permissions */
                                                    SP_CHAR1_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },        

        // Characteristic 4 client characteristic configuration
        [SP_IDX_CHAR1_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    2,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                }, 
																								
        // Characteristic 1 User Description
        [SP_IDX_CHAR1_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR1_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char1_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 2 Declaration
        [SP_IDX_CHAR2_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 2 Value   
        [SP_IDX_CHAR2_VALUE]                =   {
                                                    { UUID_SIZE_16, SP_CHAR2_RX_UUID },                 /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                                             /* Permissions */
                                                    SP_CHAR2_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */	/* Can assign a buffer here, or can be assigned in the application by user */
                                                },   
        // Characteristic 2 User Description
        [SP_IDX_CHAR2_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },       /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR2_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char2_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 3 Declaration
        [SP_IDX_CHAR3_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 3 Value
        [SP_IDX_CHAR3_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR3_UUID) },                 /* UUID */
                                                    GATT_PROP_WRITE,                                            /* Permissions */
                                                    SP_CHAR3_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 3 User Description
        [SP_IDX_CHAR3_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR3_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char3_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 4 Declaration
        [SP_IDX_CHAR4_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 4 Value
        [SP_IDX_CHAR4_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR4_UUID) },                 /* UUID */
                                                    GATT_PROP_WRITE | GATT_PROP_NOTI,                           /* Permissions */
                                                    SP_CHAR4_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },  
        // Characteristic 4 client characteristic configuration
        [SP_IDX_CHAR4_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
                                                    SP_CHAR4_CCC_LEN,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                }, 
        // Characteristic 4 User Description
        [SP_IDX_CHAR4_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR4_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char4_desc,                                   /* Value of the attribute */
                                                },


        // Characteristic 5 Declaration 
        [SP_IDX_CHAR5_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 5 Value
        [SP_IDX_CHAR5_VALUE]                =   {
                                                    { UUID_SIZE_2, UUID16_ARR(SP_CHAR5_UUID) },                 /* UUID */
                                                    GATT_PROP_WRITE | GATT_PROP_READ,             /* Permissions */
                                                    SP_CHAR5_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        // Characteristic 5 User Description
        [SP_IDX_CHAR5_USER_DESCRIPTION]     =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    SP_CHAR5_DESC_LEN,                                          /* Max size of the value */
                                                    (uint8_t *)sp_char5_desc,                                   /* Value of the attribute */
                                                },
};
//帧头检验
void headcheck(uint8_t *data,uint32_t len) {

    if(len == 0 ) return;
    if(data[0]==0xAA && data[1]==0xEE)
    {
        ble_flag=1;
				
    }
}



// 校验和
uint8_t checksum(uint8_t *data, int length) {
    unsigned char sum = 0;
    int i;
    for (i = 0; i < length-1; i++) {
        sum += data[i];
    }
	 printf("Checksum: 0x%02X\n\r", sum);
    return sum;
}


static void show_reg(uint8_t *data,uint32_t len,uint8_t dbg_on)
{
    uint32_t i=0;
    if(len == 0 || (dbg_on==0)) return;
    for(; i<len; i++)
    {
        co_printf("0x%02X,",data[i]);
    }
		co_printf("\r\n");// 这里做蓝牙指令判断

//    co_printf("\r\n");
}
void ntf_data(uint8_t con_idx,uint8_t att_idx,uint8_t *data,uint16_t len)
{
		gatt_ntf_t ntf_att;
		ntf_att.att_idx = att_idx;
		ntf_att.conidx = con_idx;
		ntf_att.svc_id = sp_svc_id;
		ntf_att.data_len = len;
		ntf_att.p_data = data;
		gatt_notification(ntf_att);
}





/*********************************************************************
 * @fn      sp_gatt_msg_handler
 *
 * @brief   Simple Profile callback funtion for GATT messages. GATT read/write
 *			operations are handeled here.
 *
 * @param   p_msg       - GATT messages from GATT layer.
 *
 * @return  uint16_t    - Length of handled message.
 */
//蓝牙读取函数
static uint16_t sp_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
				{
/*********************************************************************
 * @brief   Simple Profile user application handles read request in this callback.
 *			应用层在这个回调函数里面处理读的请求。
 *
 * @param   p_msg->param.msg.p_msg_data  - the pointer to read buffer. NOTE: It's just a pointer from lower layer, please create the buffer in application layer.
 *					  指向读缓冲区的指针。 请注意这只是一个指针，请在应用程序中分配缓冲区. 为输出函数, 因此为指针的指针.
 *          p_msg->param.msg.msg_len     - the pointer to the length of read buffer. Application to assign it.
 *                    读缓冲区的长度，用户应用程序去给它赋值.
 *          p_msg->att_idx - index of the attribute value in it's attribute table.
 *					  Attribute的偏移量.
 *
 * @return  读请求的长度.
 */					
					
					if(p_msg->att_idx == SP_IDX_CHAR1_VALUE)
					{
						memcpy(p_msg->param.msg.p_msg_data, "CHAR1_VALUE", strlen("CHAR1_VALUE"));
						return strlen("CHAR1_VALUE");
					}
					else if(p_msg->att_idx == SP_IDX_CHAR2_VALUE)
					{
						switch(ble_id)
						{
							case IMEI:
							{
								memcpy(p_msg->param.msg.p_msg_data,"imeirev:", strlen("imeirev:"));
								return strlen("imeirev:");								
							}
						//	break;
							
							case READ:
							{ 

						     memcpy(p_msg->param.msg.p_msg_data, TX_read, strlen(TX_read));
						     return strlen(TX_read);										
							}
						//	break;
							
							case CHECK:
							{
						     memcpy(p_msg->param.msg.p_msg_data, TX_check, strlen(TX_check));
						     return strlen(TX_check);										
							}
						//	break;						

							case BATT:
							{
						     memcpy(p_msg->param.msg.p_msg_data, "BATT", strlen("BATT"));
						     return strlen("BATT");										
							}
							//break;		

							case VISON:
							{
						     memcpy(p_msg->param.msg.p_msg_data, "VISON", strlen("VISON"));
						     return strlen("VISON");										
							}
							//break;	
							
							case TIME:
							{
						     memcpy(p_msg->param.msg.p_msg_data, timesend, strlen(timesend));
						     return strlen(timesend);										
							}
							//break;				

							case CLEAN:
							{
						     memcpy(p_msg->param.msg.p_msg_data, "CLEAN", strlen("CLEAN"));
						     return strlen("CLEAN");										
							}
							//break;		
							
							case ANS:
							{
//								cn++;
//								if(cn<=jump.package-1)
//								{
									memcpy(p_msg->param.msg.p_msg_data, TX_ack, strlen(TX_ack));
									return strlen(TX_ack);	
//								}
//								else{
//									memcpy(p_msg->param.msg.p_msg_data, "Finish", strlen("Finish"));
//									cn=0;
////									flash_erase(flash_jump,0x2F000);
//									return strlen("Finish");	
//								}
//						     memcpy(p_msg->param.msg.p_msg_data, "READ", strlen("READ"));
						     													
							}
						//	break;

							default:
							{
                 break;
							}								
						}
						

					}
					else if(p_msg->att_idx == SP_IDX_CHAR4_CFG)
					{
						memcpy(p_msg->param.msg.p_msg_data, sp_char4_ccc, 2);
						return 2;
					}					
					else if(p_msg->att_idx == SP_IDX_CHAR5_VALUE)
					{
						memcpy(p_msg->param.msg.p_msg_data, "CHAR5_VALUE", strlen("CHAR5_VALUE"));
						return strlen("CHAR5_VALUE");
					}
				}
            break;
        
        case GATTC_MSG_WRITE_REQ:
				{
/*********************************************************************
 * @brief   Simple Profile user application handles write request in this callback.
 *			应用层在这个回调函数里面处理写的请求。
 *
 * @param   p_msg->param.msg.p_msg_data   - the buffer for write
 *			              写操作的数据.
 *					  
 *          p_msg->param.msg.msg_len      - the length of write buffer.
 *                        写缓冲区的长度.
 *          att_idx     - index of the attribute value in it's attribute table.
 *					      Attribute的偏移量.
 *
 * @return  写请求的长度.
 */				
					if (p_msg->att_idx == SP_IDX_CHAR1_VALUE)
					{
						co_printf("char1_recv:");
						show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
					}
					else if (p_msg->att_idx == SP_IDX_CHAR2_VALUE)
					{

						headcheck(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len); //帧头检验
						//头检验和校验位检验
						if(ble_flag==1 && (checksum(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len)==p_msg->param.msg.p_msg_data[p_msg->param.msg.msg_len-1])){
							switch(p_msg->param.msg.p_msg_data[4])
							{
								case 0x00:   //设备重启
								{
									co_printf("reboot\r\n");
								}
								break;
								
								case 0x01:    //时间设置
								{
									ble_id=TIME;
									ble_at=6;
									co_printf("timeset\r\n");
									timeble32=(p_msg->param.msg.p_msg_data[5] << 24) | (p_msg->param.msg.p_msg_data[6] << 16) | (p_msg->param.msg.p_msg_data[7] << 8) | p_msg->param.msg.p_msg_data[8];
									timestart32=timeble32;
									uart_at(NULL);
								}
								break;
								
								case 0x02:  //基础信息设置--0x02
								{
									co_printf("baseset\r\n");
								}
								break;		
								 
								case 0x03:   //数据清除--0x03
								{
									ble_id=CLEAN;
									co_printf("clean\r\n");
								}
								break;							

								case 0xA0:  //数据条数查询--0xA0
								{
									ble_id=CHECK;
									co_printf("check\r\n");
									ble_at=3;
									uart_at(NULL);
								}
								break;			

								case 0xA1:  //数据读取指令--0xA1
								{
									ble_id=READ;
									co_printf("read\r\n");
//									ble_at=1;
									ble_at=4;
									uart_at(NULL);
								}
								break;								
								
								case 0xA2:  // 版本获取--0XA2
								{
									ble_id=VISON;
									co_printf("vision\r\n");
								}
								break;
								
								case 0xA3:  //Imei 获取--0XA3
								{
								  
									
									co_printf("imei\r\n");
			            ble_at=2;//at_imei
			            uart_at(NULL);
									
									
									ble_id=IMEI;
								}
								break;
								
								case 0xA4:  //电量获取--0XA4
								{
									ble_id=BATT;
									co_printf("battery\r\n");
								}
								break;
								
								case 0xA5: //应答
								{
									ble_id=ANS;
									co_printf("ack\r\n");
									ble_at=5;
									uart_at(NULL);
								}
								break;										
								
								default:
								{
									co_printf("error\r\n");
								}
								break;
							}
						}
					show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);

						
					}
					else if (p_msg->att_idx == SP_IDX_CHAR3_VALUE)
					{
						co_printf("char3_recv:");
						show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
					}
					else if (p_msg->att_idx == SP_IDX_CHAR5_VALUE)
					{
						co_printf("char5_recv:");
						show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
					}
					else if (p_msg->att_idx == SP_IDX_CHAR1_CFG)
					{
						
					
						co_printf("char1_ntf_enable:MTU is %d\r\n",gatt_get_mtu(p_msg->conn_idx));
						
					
					
						//flash_read(flash_imei,sizeof(imeirev),(uint8_t *)imeirev);	
					
						show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
			
						if(1){
							//ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)"char1_ntf_data",strlen("char1_ntf_data"));
//						
						switch(ble_id)
						{
							case IMEI:   //Imei 获取--0XA3
							{
									
								//flash_write(flash_imei,100,(uint8_t *)imeireva);
							
								flash_read(flash_imei,15,&uint8_imei);
								send_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,&uint8_imei,15,0xa3);
							//	send_packet(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,0x01,0x01,0xa3,&uint8_imei);
								//ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,&uint8_imei,15);
								//ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)imeirev,strlen(imeirev));
													
							}
							break;
							
							case READ:
							{ 
								uint8_t  *read_a1;
								int  AT_cout=3*jump.count+15;
								read_a1 = (uint8_t *)os_malloc( AT_cout * sizeof(uint8_t));	
								flash_read(lanya_a1_save,AT_cout,(uint8_t *)read_a1);
								send_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)read_a1,AT_cout,0xa1);
								
								os_free(read_a1);
								

							}
								
							
							break;
							
							case CHECK:  //电量获取--0XA4
							{
								ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)TX_check,strlen(TX_check));
//						     memcpy(p_msg->param.msg.p_msg_data, TX_check, strlen(TX_check));
//						     return strlen(TX_check);										
							}
							break;						

							case BATT:
							{
								ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)"BATT",strlen("BATT"));
//						     memcpy(p_msg->param.msg.p_msg_data, "BATT", strlen("BATT"));
//						     return strlen("BATT");										
							}
							break;		

							case VISON:
							{
								ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)"VISON",strlen("VISON"));
//						     memcpy(p_msg->param.msg.p_msg_data, "VISON", strlen("VISON"));
//						     return strlen("VISON");										
							}
							break;	
							
							case TIME:  //时间设置
							{
								
								ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)timesend,strlen(timesend));
//						     memcpy(p_msg->param.msg.p_msg_data, timesend, strlen(timesend));
//						     return strlen(timesend);										
							}
							break;				

							case CLEAN:
							{
								ntf_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)"CLEAN",strlen("CLEAN"));
//						     memcpy(p_msg->param.msg.p_msg_data, "CLEAN", strlen("CLEAN"));
//						     return strlen("CLEAN");										
							}
							break;		
							
							case ANS: //应答
							{ 
								

								at_flash_read=1;
								printf("this is lanya ,nb_count is %d\r\n",nb_count);
								if(nb_count!=0)
								{
									int num;
									uint8_t  *read_a1;
									int i;
									for(i=0;i<nb_count;i++)
									{
									int at_count;
									num=flash_jump+(LANYA_FLASH*i);
									read_a1 = (uint8_t *)os_malloc(LANYA_FLASH* sizeof(uint8_t));	
									flash_read(num,LANYA_FLASH,(uint8_t *)read_a1);
									at_count=read_a1[0] << 8 | read_a1[1];
									if(at_count>LANYA_FLASH)
									{
										at_count=15;
									}
									printf("at_count is %d\r\n",at_count);
									send_data(p_msg->conn_idx,SP_IDX_CHAR1_VALUE,(uint8_t *)read_a1,at_count,0xa5);
									
									os_free(read_a1);

									}
							}
								nb_count=0;
								flash_nb_write(nb_count);
								flash_erase(flash_jump,0x10000);
								co_printf("flash_jump has been cleaned in lanya\r\n");

						   at_flash_read=0;  													
							}
							break;

							default:
							{
                 break;
							}								
						}
					}
//						
					}
					else if (p_msg->att_idx == SP_IDX_CHAR4_CFG)
					{
						co_printf("char4_ntf_enable:");
						show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
						memcpy(sp_char4_ccc,p_msg->param.msg.p_msg_data,2);
						
					}
				}
            break;
        case GATTC_MSG_LINK_CREATE:
						co_printf("link_created\r\n");
            break;
        case GATTC_MSG_LINK_LOST:
						co_printf("link_lost\r\n");
						ntf_char1_enable = 0;
            break;    
        default:
            break;
    }
    return p_msg->param.msg.msg_len;
}

/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *			添加GATT service到ATT的数据库里面。
 *
 * @param   None. 
 *        
 *
 * @return  None.
 */
void sp_gatt_add_service(void)
{
	simple_profile_svc.p_att_tb = simple_profile_att_table;
	simple_profile_svc.att_nb = SP_IDX_NB;
	simple_profile_svc.gatt_msg_handler = sp_gatt_msg_handler;
	
	sp_svc_id = gatt_add_service(&simple_profile_svc);
}





