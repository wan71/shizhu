#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "driver_gpio.h"
#include "user_task.h"

#define USE_HORIZONTAL 1  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏



#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 80
#endif



//-----------------LCD端口定义---------------- 

#define LCD_SCLK_Clr() gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_4,0)//SCL=SCLK
#define LCD_SCLK_Set() gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_4,1)

#define LCD_MOSI_Clr() gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_6,0)//SDA=MOSI
#define LCD_MOSI_Set() gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_6,1)

#define LCD_RES_Clr()  gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_0,0)//RES
#define LCD_RES_Set()  gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_0,1)

#define LCD_DC_Clr()   gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_7,0)//DC
#define LCD_DC_Set()   gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_7,1)

#define LCD_CS_Clr()   gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_5,0)//CS
#define LCD_CS_Set()   gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_5,1)

#define LCD_BLK_Clr()  gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_1,1)//BLK
#define LCD_BLK_Set()  gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_1,0)




void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif
