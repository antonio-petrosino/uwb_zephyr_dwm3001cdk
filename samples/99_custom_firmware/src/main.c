#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <deca_probe_interface.h>
#include <deca_device_api.h>
#include <port.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <../include/sit_led/sit_led.h>
#include <../include/sit/sit_device.h>
#include <../include/sit/sit_config.h>
#include <sit/sit.h>


#include </home/ubuntu/sit/include/sit/sit.h>
#include "sit/sit_distance.h"
#include "sit/sit_utils.h"



LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED0_NODE 	DT_ALIAS(led0)
#define LED1_NODE 	DT_ALIAS(led1)
#define LED2_NODE 	DT_ALIAS(led2)
#define LED3_NODE 	DT_ALIAS(led3)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385


void main(void)
{
	printk("Welcome to the Custom Firmware Test\n");
	LOG_INF("SS TWR INIT v1.0\n");

	//int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);

	sit_led_init();
	
	//int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);
	//set_device_state("start");

	uint8_t init_status = sit_init_custom();
	
	if(init_status > 1)
	{
	LOG_INF("=========== sit_init OK =======\n");
	}else{
	LOG_INF("=========== sit_init KO! =======\n");
	}
	
	
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	gpio_pin_set_dt(&led0, 0);
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);
	gpio_pin_set_dt(&led3, 0);
	k_sleep(K_MSEC(1000));
	gpio_pin_set_dt(&led0, 1);
	gpio_pin_set_dt(&led1, 1);
	gpio_pin_set_dt(&led2, 1);
	gpio_pin_set_dt(&led3, 1);
	k_sleep(K_MSEC(1000));
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
	

	int count = 0;
	while (1) {
		count++;
		LOG_INF("=========== ciclo %d =======\n", count);
		LOG_INF("leds> hello %d",count);
		
		/*gpio_pin_set_dt(&led1, 0);
		k_sleep(K_MSEC(300));
		gpio_pin_set_dt(&led1, 1);
		gpio_pin_set_dt(&led0, 0);
		k_sleep(K_MSEC(300));
		gpio_pin_set_dt(&led0, 1);
		gpio_pin_set_dt(&led2, 0);
		k_sleep(K_MSEC(300));
		gpio_pin_set_dt(&led2, 1);
		gpio_pin_set_dt(&led3, 0);
		k_sleep(K_MSEC(300));
		gpio_pin_set_dt(&led3, 1);
		*/

		k_sleep(K_MSEC(1000));
	}
}
