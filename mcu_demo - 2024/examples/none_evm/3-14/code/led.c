#include "led.h"
#include "driver_gpio.h"

void LED_Init(void)
{
   system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_A0);
	 system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
	 system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_A4);
	 system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_A5);
	 system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_A6);
	 system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_A7);
	
	 gpio_porta_set_dir(0);
} 
