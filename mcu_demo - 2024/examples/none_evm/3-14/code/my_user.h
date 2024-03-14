#ifndef MY_USER_H
#define MY_USER_H
#include <stdint.h>  
#include "user_task.h"

void flash_nb_init(void);
uint16_t flash_nb_read(void);
void flash_nb_write(uint16_t nb_count);
void flash_change_nb(int num);
char *formatJumpData(const JumpData *data,int a,int i);
void pack_data(indexdata *data, size_t num_elements, uint8_t *packed_data);
void jump_init(JumpData *jump);
void strmncpy(char *s, int m, int n, char *t);
void convertToSixInt(const char* datetime, int* year, int* month, int* day, int* hour, int* minute, int* second);
void extract_index_dt(const char* str, char* hex_result);
int isLeapYear(int year);
int daysOfMonth(int year, int month);
void AT_send(const char *message);
void str_to_uint8_t_array(const char *str, uint8_t *array, size_t size);
void str_to_uint8_t(const char *s, int m, int n, uint8_t *result);
uint64_t getTimestamp(int year, int month, int day, int hour, int minute, int second);
uint32_t getTimestamp32(int year, int month, int day, int hour, int minute, int second);
unsigned char calculateChecksum(const char *array, size_t size);
int verify_flash_datas(uint32_t offset, uint32_t length, uint8_t *expected_data);
void flash_write_with_retry(uint32_t offset, uint32_t length, uint8_t *buffer);
void concatenateStrings(uint8_t TX1[], uint8_t TX2[], uint8_t TX[]) ;
void send_packet(uint8_t con_idx, uint8_t att_idx, uint8_t frame_number, uint8_t total_frames, uint8_t command, uint8_t *data);
void send_data(uint8_t con_idx, uint8_t att_idx, uint8_t *data, uint16_t len,uint8_t command);
void writer_verify_flash_datas(uint32_t offset, uint32_t length, uint8_t *buffer);
#endif  // _USER_TASK_H

