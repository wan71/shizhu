#include <stdint.h>  
#include <stdbool.h>
#include "os_timer.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include "simple_gatt_service.h"
#include "ble_simple_peripheral.h"
#include "inttypes.h"
#include "sys_utils.h"
#include "flash_usage_config.h"
#include "user_task.h"
#include "driver_flash.h"
#include "driver_uart.h"
#include "os_mem.h"

extern uint8_t write_success;
#define MAX_WRITE_RETRIES   3
extern int j;
extern int dt;
#define PACKET_HEADER 0xAF
#define PACKET_HEADER_1 0xAA
#define PACKET_HEADER_2 0xFF
 // 你的命令字节，根据实际情况替换
#define PACKET_SIZE 20


// 函数用于格式化 JumpData 为 JSON 格式的字符串
char *formatJumpData(const JumpData *data,int a,int num) {
	
    char *result = (char *)os_malloc(JUMP_MALLOC*sizeof(char));  // 根据需要调整初始大小
    if (result == NULL) {
        // 处理内存分配失败
        return NULL;
    }

    // 格式化 JumpData 为 JSON 格式的字符串
    sprintf(result, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%d\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
					 data->deviceId,data->package,data->packageindex,data->jump_time,data->mode,data->count);
		
		strcat(result, "{");
		
		// 格式化 indexdata 数组
    for (int i = 0; i < a; ++i) {
        sprintf(result + strlen(result), "index:%d,dt:%d;", data->store[(package_num * num) + i].cindex, data->store[(package_num * num) + i].jump_dt);
    }
		strcat(result,"}\"}\"\r\n");

    return result;
}


//将flash的数据改成json格式
void flash_change_nb(int num)
{
	  uint8_t data_head[15];
		uint8_t *data;
	  JumpData jump_2;
   	int at_count;
	  int m;
	  int n=0;
	  
		//int packageindex,package;
		flash_read(num,15,(uint8_t *)data_head);
		at_count=data_head[0] << 8 | data_head[1];
		if(at_count>LANYA_FLASH)
		{
			at_count=15;
		}
		data = (uint8_t *)os_malloc(at_count* sizeof(uint8_t));	
		flash_read(num,at_count,(uint8_t *)data);
		printf("at_count is %d\r\n",at_count);
		
		at_count=at_count-15;  //去调头的数据
		if(at_count>0)
		{
	
		//jump_2初始化
		jump_2.count=at_count/3;
		jump_2.package=1+jump_2.count/package_num;
		jump_2.packageindex=1;	
	  jump_2.deviceId="123";
		jump_2.mode=1;
		int a=0;
		jump_2.jump_time= data_head[6] << 24 | data_head[7]<<16 | data_head[8] << 8 | data_head[9];
		printf("jump_2.count is %d\r\n",jump_2.count);
			
		for(m=0;m<jump_2.count;m++)
			{ 

				uint8_t jump_dt=data[m * 3+15];
				uint16_t cindex=data[m * 3+16]<<8|data[m * 3+17];
				jump_2.store[m].jump_dt=jump_dt;
				jump_2.store[m].cindex=cindex;
		
			}
			
			char formattedString[580]={0};
			while(jump_2.package!=n)
			{
				
			sprintf(formattedString, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%d\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
					 jump_2.deviceId,jump_2.package,jump_2.packageindex,jump_2.jump_time,jump_2.mode,jump_2.count);
		  
				int sum=n*package_num;
			if(n==jump_2.package-1)
			{
				
				a=jump_2.count-sum;
			}
			else{
						a=package_num;
					 }
			
				
			 // 格式化 indexdata 数组
				for (int i = 0; i < a; ++i) {
						sprintf(formattedString + strlen(formattedString), "index:%d,dt:%d;", jump_2.store[sum+i].cindex, jump_2.store[sum+i].jump_dt);
				}
				strcat(formattedString,"}\"}\"\r\n");
				uart_write(UART0,(uint8_t *)formattedString, strlen((char*)formattedString));	
			  printf("%s\n", formattedString);
				co_delay_100us(100);
				n++;
				jump_2.packageindex++;
				
			}
			
//				for(n=0;n < jump_2.package;n++)
//				{  
//					printf("n is %d\r\n",n);
//				
//					char formattedString[5]={0};
//					int a;
//					//co_printf("i is %d,c is %d\r\n",i,nb_count);		
//	
//					if(n==jump_2.package-1)
//					{
//						a=jump_2.count-(n*package_num);
//					}
//					else{
//						a=package_num;
//					 }
//					
//			sprintf(formattedString, "AT+LCTM2MSEND=1,\"{\"serviceId\": 666, \"deviceId\":\"%s\", \"package\":\"%d\", \"packageIndex\":\"%d\", \"jtime\":\"%d\", \"mode\":\"%d\", \"count\":\"%d\", \"data\":\"",
//					 jump_2.deviceId,jump_2.package,jump_2.packageindex,jump_2.jump_time,jump_2.mode,jump_2.count);
//					 
//					 // 格式化 indexdata 数组
//				for (int i = 0; i < a; ++i) {
//						sprintf(formattedString + strlen(formattedString), "index:%d,dt:%d;", jump_2.store[(package_num * n) +i].cindex, jump_2.store[(package_num * n) +i].jump_dt);
//				}
//				strcat(formattedString,"}\"}\"\r\n");
//				uart_write(UART0,(uint8_t *)formattedString, strlen((char*)formattedString));	
//				printf("%s\n", formattedString);
//				jump_2.packageindex++;
//				
//					}


		}
		os_free(data);
		
}





void send_packet(uint8_t con_idx, uint8_t att_idx, uint8_t frame_number, uint8_t total_frames, uint8_t command, uint8_t *data) {
    uint8_t packet_data[PACKET_SIZE];
    packet_data[0] = PACKET_HEADER;
    packet_data[1] = total_frames;
    packet_data[2] = command;
    packet_data[3] = frame_number;
  //  memcpy(&packet_data[5], data, 14);
    memcpy(&packet_data[4], data, 15);

    // 计算校验字节，这里使用了简单的累加
    uint8_t checksum = 0;
    for (int i = 0; i < PACKET_SIZE - 1; i++) {
        checksum += packet_data[i];
    }
    packet_data[19] = checksum;

    // 调用 ntf_data 发送分包数据
    ntf_data(con_idx, att_idx, packet_data, PACKET_SIZE);
		co_delay_100us(10);
}

void send_data(uint8_t con_idx, uint8_t att_idx, uint8_t *data, uint16_t len,uint8_t command) {
    uint8_t total_frames = (len + 14) / 15; // 计算总的帧数，每个帧包含14字节数据（不包括包头）
    uint8_t frame_number = 1;

    for (int i = 0; i < len; i += 15) {
        uint8_t remaining_bytes = len - i;
        uint8_t frame_data[15];

        if (remaining_bytes >= 15) {
            memcpy(frame_data, &data[i], 15);
        } else {
            memcpy(frame_data, &data[i], remaining_bytes);
            memset(&frame_data[remaining_bytes], 0, 15 - remaining_bytes); // 填充剩余的字节为 0
        }

        send_packet(con_idx, att_idx, frame_number, total_frames, command, frame_data);
        frame_number++;
    }
}




/*
void send_packet(uint8_t con_idx, uint8_t att_idx, uint8_t frame_number, uint8_t total_frames, uint8_t command, uint8_t *data) {
    uint8_t packet_data[PACKET_SIZE];
    packet_data[0] = PACKET_HEADER_1;
    packet_data[1] = PACKET_HEADER_2;
    packet_data[2] = frame_number;
    packet_data[3] = total_frames;
    packet_data[4] = command;
    memcpy(&packet_data[5], data, 14);
   // memcpy(&packet_data[4], data, 15);

    // 计算校验字节，这里使用了简单的累加
    uint8_t checksum = 0;
    for (int i = 0; i < PACKET_SIZE - 1; i++) {
        checksum += packet_data[i];
    }
    packet_data[19] = checksum;

    // 调用 ntf_data 发送分包数据
    ntf_data(con_idx, att_idx, packet_data, PACKET_SIZE);
		co_delay_100us(10);
}

void send_data(uint8_t con_idx, uint8_t att_idx, uint8_t *data, uint16_t len,uint8_t command) {
    uint8_t total_frames = (len + 13) / 14; // 计算总的帧数，每个帧包含14字节数据（不包括包头）
    uint8_t frame_number = 1;

    for (int i = 0; i < len; i += 14) {
        uint8_t remaining_bytes = len - i;
        uint8_t frame_data[14];

        if (remaining_bytes >= 14) {
            memcpy(frame_data, &data[i], 14);
        } else {
            memcpy(frame_data, &data[i], remaining_bytes);
            memset(&frame_data[remaining_bytes], 0, 14 - remaining_bytes); // 填充剩余的字节为 0
        }

        send_packet(con_idx, att_idx, frame_number, total_frames, command, frame_data);
        frame_number++;
    }
}

*/




//自己加的
// 校验写入的数据是否正确
int verify_flash_datas(uint32_t offset, uint32_t length, uint8_t *expected_data) {
   // uint8_t read_data[JUMP_MALLOC];
	  uint8_t *read_data = (uint8_t *)os_malloc(length*sizeof(uint8_t));
		uint32_t i;
    // 读取 Flash 数据
    flash_read(offset, length, read_data);

    // 比较读取的数据和期望的数据
    for (i = 0;i < length;i++) {
        if (read_data[i] != expected_data[i]) {
            return 0;  // 数据不一致，校验失败
        }
    }
		os_free(read_data);

    return 1;  // 数据一致，校验成功
}

void flash_write_with_retry(uint32_t offset, uint32_t length, uint8_t *buffer) {
	    int retry_count;
    for (retry_count = 0; retry_count < MAX_WRITE_RETRIES; retry_count++) {
        // 尝试写入
        flash_write(offset, length, buffer);

        // 检查写入是否成功
        // 这里假设 flash_write 函数返回一个成功或失败的标志
        if (verify_flash_datas(offset,length,buffer)) {
            co_printf("Write succeeded!\r\n");
             // 写入成功，退出函数
        } else {
            co_printf("Write attempt %d failed. Retrying...\r\n", retry_count+1 );
            // 写入失败，等待一段时间再进行下一次尝试
            co_delay_100us(10000);
        }
    }

    co_printf("Max retries reached. Failed to write.\r\n");
}



void writer_verify_flash_datas(uint32_t offset, uint32_t length, uint8_t *buffer)
{
		flash_erase(offset,length);
	flash_write(offset,length,buffer);
		
	if (verify_flash_datas(offset,length,buffer)) {
			// 校验成功，数据写入正确
			//co_printf("writer is succss!!\r\n");
	} else {
			// 校验失败，数据写入错误
		 // 调用时使用 flash_write_with_retry
			flash_write_with_retry(offset,length,buffer);
	}
	
	
	
}


void flash_nb_write(uint16_t nb_count)
{
	uint8_t buffer[2];
  buffer[0] = (nb_count >> 8) & 0xFF; // 高字节
  buffer[1] = nb_count & 0xFF;        // 低字节
	// 调用 flash_write 函数将数据写入 Flash 存储器
  writer_verify_flash_datas(flash_count, sizeof(buffer), buffer);

}


uint16_t flash_nb_read(void)
{
	// 缓冲区用于存储从 Flash 读取的数据
    uint8_t buffer[2];
	// 调用 flash_read 函数从 Flash 存储器中读取数据
    flash_read(flash_count, sizeof(buffer), buffer);

    // 将缓冲区中的数据组合成 uint16_t
    uint16_t readData = (buffer[0] << 8) | buffer[1];

    // 打印读取到的 uint16_t 数据
    co_printf("nb_count: %d\r\n", readData);
	
    return readData;
	
}

void flash_nb_init(void)
{
	
	flash_nb_write(0);
	
}


void str_to_uint8_t(const char *s, int m, int n, uint8_t *result) {
    int k = 0;

    // Find the length of the string
    while (s[k] != '\0') {
        k++;
    }

    // If the string length is less than or equal to m, set the result to an empty array
    if (k <= m) {
        *result = 0;
        return;
    }

    // Move the pointer to the starting position
    const char *p = s + m;

    // Copy n characters to the uint8_t array
    for (int i = 0; i < n; i++) {
        *result = (uint8_t)(*p);
        
        // If the character is '\0', stop copying
        if (*result == '\0') {
            return;
        }

        result++;
        p++;
    }

    // Set the last element to 0 to terminate the uint8_t array
    *result = 0;
}


void str_to_uint8_t_array(const char *str, uint8_t *array, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            array[i] = (uint8_t)(str[i] - '0');
        } else {
            // Handle the case where the character is not a digit
            // You can add error handling or default values as needed
            array[i] = 0;
        }
    }
}

/*
const char *myMessage = "AT+CGSN=1\r\n";
AT_send(myMessage);
*/
void AT_send(const char *message)
{
uart_write(UART0,(const uint8_t *)message, strlen(message));	
co_printf("Sending message: %s\n", message);
}


void jump_init(JumpData *jump)  //jump_init清零
{ 
		j=0;
	  dt=0;
		jump->count=0;
		jump->package=1;
		jump->packageindex=1;	
	  jump->deviceId="123";
		jump->mode=1;
		
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


// 计算数组的异或校验位
char calculateChecksum(const char *array, size_t size) {
    char checksum = 0;

    for (size_t i = 0; i < size; i++) {
        checksum ^= array[i];
    }

    return checksum;
}




