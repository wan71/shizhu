#ifndef _USER_TASK_H
#define _USER_TASK_H

#include <stdint.h>  

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t


#define at_package_num 5
#define flash_jump 0x65000  
#define flash_imei 0x50000
#define package_num  30//һ�ΰ��ĸ���
#define lanya_a1_save 0x64000
#define flash_count 0x54000
#define JUMP_MALLOC 512
#define LANYA_FLASH 1024
typedef unsigned int size_t;

typedef struct {
   uint8_t jump_dt;
   uint16_t cindex;
} indexdata;



typedef struct {
	 char *deviceId;
	 int package;
	 int packageindex;
   uint32_t jump_time;  
	 uint16_t  mode;
	 uint16_t count;
   indexdata store[512];	 
} JumpData;

enum user_event_t {
    USER_EVT_AT_COMMAND,
    USER_EVT_BUTTON,
	  UART_AT,
	  AT_UART,
		HUOER_INTE,
	  UART_READ,
	FLASH_SAVE,
	FLASH_READ,
};


extern uint16_t user_task_id;

void user_task_init(void);

void uart_at(void *param);
void oled_init(void);
void at_uart_init(char *AT_uart_msg);
void flash_read_start(void *param);
void flash_save_init(void);
void flash_save(void *param);
int kanya_save(int save_palce);
void flash_num_write(uint8_t num);
void huoer_inte(void *param);

uint16_t TX_NUM(indexdata a[]);

void generateString(int num,int a, indexdata b[],uint8_t TX[]) ;

void element_verify(uint32_t interupt);


#endif  // _USER_TASK_H
