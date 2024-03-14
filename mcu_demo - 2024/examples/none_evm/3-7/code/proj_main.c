/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
#include "button.h"
#include "jump_table.h"

#include "user_task.h"

#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_uart.h"
#include "driver_rtc.h"
#include "driver_flash.h"
#include "driver_efuse.h"
#include "driver_gpio.h"
#include "flash_usage_config.h"

#include "ble_simple_peripheral.h"
#include "simple_gatt_service.h"

#include "lcd_init.h"
#include "lcd.h"
#include "led.h"



/*
 * LOCAL VARIABLES
 */

const struct jump_table_version_t _jump_table_version __attribute__((section("jump_table_3"))) = 
{
    .firmware_version = 0x00000000,
};

const struct jump_table_image_t _jump_table_image __attribute__((section("jump_table_1"))) =
{
    .image_type = IMAGE_TYPE_APP,
    .image_size = 0x19000,      
};


__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
	 //�Լ���
	 //uint32_t gpio_status_reg = ool_read32(PMU_GPIO_STATUS_REGISTER); // ��ȡ GPIO ״̬�Ĵ���
	 uint32_t pmu_int_pin_setting = ool_read32(PMU_REG_PORTA_TRIG_MASK); 
	// ��ȡ PMU_REG_PORTA_TRIG_MASK �Ĵ�����ֵ
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);
	// ��ȡ PMU_REG_GPIOA_V �Ĵ�����ֵ
    
    button_toggle_detected(gpio_value);
	// ��ⰴť״̬�ı�
    ool_write32(PMU_REG_PORTA_LAST, gpio_value);
	// �� gpio_value ��ֵд�� PMU_REG_PORTA_LAST �Ĵ���
	
	
	uint32_t tmp = gpio_value & pmu_int_pin_setting;
	// �� gpio_value �� pmu_int_pin_setting ����λ����
	uint32_t pressed_key = tmp^pmu_int_pin_setting;
	/*
	co_printf("*****************************\r\n\n\n");
	co_printf("pmu_int_pin_setting:0x%08x\r\n", pmu_int_pin_setting);
	co_printf("gpio_value:0x%08x\r\n", gpio_value);
	co_printf("tmp:0x%08x\r\n", tmp);
	co_printf("pressed_key:0x%08x\r\n", pressed_key);
	co_printf("*****************************\r\n\n\n");
	*/
	// ��ӡ���µİ���ֵ
	
	//o_printf("K:0x%08x\r\n", (pressed_key));  //�Լ��ӵ�
	switch(pressed_key)
	{
		case 0x00300000:
		{
			
			flash_save(NULL);
			
		}
		break;
		case 0x00d00000:
		{
			
			element_verify(pressed_key);
			
		}
		break;
		
		
		default:
			break;
	}
		
			

}

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    struct chip_unique_id_t id_data;

    efuse_get_chip_unique_id(&id_data);
    __jump_table.addr.addr[0] = 0xBD;
    __jump_table.addr.addr[1] = 0xAD;
    __jump_table.addr.addr[2] = 0xD0;
    __jump_table.addr.addr[3] = 0xF0;
    __jump_table.addr.addr[4] = 0x17;
    __jump_table.addr.addr[5] = 0x20;
    
    id_data.unique_id[5] |= 0xc0; // random addr->static addr type:the top two bit must be 1.
    memcpy(__jump_table.addr.addr,id_data.unique_id,6);
    __jump_table.system_clk = SYSTEM_SYS_CLK_48M;
    jump_table_set_static_keys_store_offset(JUMP_TABLE_STATIC_KEY_OFFSET);
    ble_set_addr_type(BLE_ADDR_TYPE_PUBLIC);
    retry_handshake();
}

/*********************************************************************
 * @fn      user_entry_before_sleep_imp
 *
 * @brief   Before system goes to sleep mode, user_entry_before_sleep_imp()
 *          will be called, MCU peripherals can be configured properly before 
 *          system goes to sleep, for example, some MCU peripherals need to be
 *          used during the system is in sleep mode. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
	uart_putc_noint_no_wait(UART1, 's');
}

/*********************************************************************
 * @fn      user_entry_after_sleep_imp
 *
 * @brief   After system wakes up from sleep mode, user_entry_after_sleep_imp()
 *          will be called, MCU peripherals need to be initialized again, 
 *          this can be done in user_entry_after_sleep_imp(). MCU peripherals
 *          status will not be kept during the sleep. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
    /* set PA2 and PA3 for AT command interface */
    //system_set_port_pull(GPIO_PA2, true);
    //system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    
    uart_init(UART1, BAUD_RATE_115200);
    //NVIC_EnableIRQ(UART1_IRQn);
		uart_putc_noint_no_wait(UART1, 'w');
    // Do some things here, can be uart print

    NVIC_EnableIRQ(PMU_IRQn);
}

void uarst_init(void)
{
	
    // Enable UART print.
    system_set_port_pull(GPIO_PD4, true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTA4_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTA5_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_9600); 
	
	  system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    //uart_init(UART1, BAUD_RATE_9600); 
		uart_init(UART1,BAUD_RATE_115200);
	
	  NVIC_EnableIRQ(UART0_IRQn);
		NVIC_EnableIRQ(UART1_IRQn);
		
	
};

void element_init(void)
{
	pmu_set_pin_to_PMU(GPIO_PORT_C,(1<< GPIO_BIT_6)|(1<< GPIO_BIT_7));
	
	pmu_set_port_mux(GPIO_PORT_C,(1<< GPIO_BIT_6)|(1<< GPIO_BIT_7), PMU_PORT_MUX_GPIO);
	
	pmu_set_pin_dir(GPIO_PORT_C,(1<< GPIO_BIT_6)|(1<< GPIO_BIT_7), GPIO_DIR_IN);//GPIO_DIR_IN
	
 pmu_set_pin_pull(GPIO_PORT_C,(1<< GPIO_BIT_7),true);//����
 pmu_set_pin_pull(GPIO_PORT_C,(1<< GPIO_BIT_6),true);
	
	
	
	
};

void key_init(void)
{
		pmu_set_pin_to_PMU(GPIO_PORT_C,(1<< GPIO_BIT_5));
	
	  pmu_set_port_mux(GPIO_PORT_C,(1<< GPIO_BIT_5), PORTC5_FUNC_C5);
	
		pmu_set_pin_dir(GPIO_PORT_C,(1<< GPIO_BIT_5), GPIO_DIR_IN);//GPIO_DIR_IN
	
		pmu_set_pin_pull(GPIO_PORT_C,(1<< GPIO_BIT_5),true);
 
};


/*********************************************************************
 * @fn      user_entry_before_ble_init
 *
 * @brief   Code to be executed before BLE stack to be initialized.
 *          Power mode configurations, PMU part driver interrupt enable, MCU 
 *          peripherals init, etc. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
//������ʼ��ǰ����
//һЩ����������������� �簴ť ���������� ��ʾ�� ����
void user_entry_before_ble_init(void)
{    
    /* set system power supply in BUCK mode */
    pmu_set_sys_power_mode(PMU_SYS_POW_BUCK);
#ifdef FLASH_PROTECT
    flash_protect_enable(1);
#endif
    pmu_enable_irq(PMU_ISR_BIT_ACOK
                   | PMU_ISR_BIT_ACOFF
                   | PMU_ISR_BIT_ONKEY_PO
                   | PMU_ISR_BIT_OTP
                   | PMU_ISR_BIT_LVD
                   | PMU_ISR_BIT_BAT
                   | PMU_ISR_BIT_ONKEY_HIGH);
    NVIC_EnableIRQ(PMU_IRQn);
	  LED_Init();//LED��ʼ��
	  LCD_Init();//LCD��ʼ��
	  LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
		uarst_init();
	 
		key_init();

		

		
		//system_set_port_pull(GPIO_PC5, true);//��ť
 		
    system_set_port_pull(GPIO_PB7, true);//ʹ��nb
		element_init(); //������ʼ��,�õ�mpu���ж�
		
		
	//	system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_C5);
		system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_B7);//����һ�����ŵ����� �������Ź��� �������ŷ�������������룩 ���õ�ƽ
		
		
    
		gpio_set_dir(GPIO_PORT_B, GPIO_BIT_7, GPIO_DIR_OUT);
		gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_7,0);
		//gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_7,1);
		
	  pmu_port_wakeup_func_set(GPIO_PC5|GPIO_PC6|GPIO_PC7); //����PMU���жϣ���Ӧ����Ϊpmu_gpio_isr_ram

		
    pmu_set_led2_value(1);
	 
}

/*********************************************************************
 * @fn      user_entry_after_ble_init
 *
 * @brief   Main entrancy of user application. This function is called after BLE stack
 *          is initialized, and all the application code will be executed from here.
 *          In that case, application layer initializtion can be startd here. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
//������ʼ������
//������ ������������
void user_entry_after_ble_init(void)
{
    co_printf("BLE Peripheral\r\n");
	
#if 1
    system_sleep_disable();		//disable sleep 
#else
    if(__jump_table.system_option & SYSTEM_OPTION_SLEEP_ENABLE)  //if sleep is enalbed, delay 3s for JLINK 
    {
        co_printf("\r\na");
        co_delay_100us(10000);       
        co_printf("\r\nb");
        co_delay_100us(10000);
        co_printf("\r\nc");
        co_delay_100us(10000);
        co_printf("\r\nd");
    }
#endif
		
    // User task initialization, for buttons.
    user_task_init();//������
	
	

    // Application layer initialization, can included bond manager init, 
    // advertising parameters init, scanning parameter init, GATT service adding, etc.    
    simple_peripheral_init();//��������
	
		oled_init();
		
}