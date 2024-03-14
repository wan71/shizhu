#ifndef _USER_TASK_H
#define _USER_TASK_H

#include <stdint.h>  

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

typedef unsigned int size_t;

typedef struct {
	
   uint32_t jump_dt;
   uint16_t cindex;
} indexdata;

typedef struct {
	 char *deviceId;
	 int package;
	 int packageindex;
   char jump_time[21];  
	 int mode;
	 uint16_t count;	
	
   indexdata store[20];
	 
} JumpData;

enum user_event_t {
    USER_EVT_AT_COMMAND,
    USER_EVT_BUTTON,
	  UART_AT,
	  AT_UART,
	  UART_READ,
};


extern uint16_t user_task_id;

void user_task_init(void);

void uart_at(void *param);

void uart_Read(void *param);

void strmncpy(char *s, int m, int n, char *t);

void convertToSixInt(const char* datetime, int* year, int* month, int* day, int* hour, int* minute, int* second);

int isLeapYear(int year);

int daysOfMonth(int year, int month);

uint64_t getTimestamp(int year, int month, int day, int hour, int minute, int second);

uint32_t getTimestamp32(int year, int month, int day, int hour, int minute, int second);

void addElevenDigitNumbers(uint64_t a, uint64_t b, char* result);

uint16_t TX_NUM(indexdata a[]);

void generateString(int a, indexdata b[],uint8_t TX[]) ;

void concatenateStrings(uint8_t TX1[], uint8_t TX2[], uint8_t TX[]) ;

void extract_index_dt(const char* str, char* hex_result);

#endif  // _USER_TASK_H

