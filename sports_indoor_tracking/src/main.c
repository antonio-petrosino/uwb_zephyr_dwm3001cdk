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
#include <sit_ble/ble_init.h>


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

device_type device;


static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    DWT_SFD_DW_8,     /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};


void main(void)
{
	// toRead = screen /dev/ttyACM0 115200
	// toCheck = mesg | grep tty
	//printk("Welcome to the Custom Firmware Test\n");
	LOG_INF("Welcome to the Custom Firmware Test\n");
	LOG_INF("SS TWR INIT or RESP v1.0\n");

	custom_configuration(config);
	initialization();
	
	init_device_id();
	char *deviceID;
	get_device_id(&deviceID);
	
	LOG_INF("=========== deviceID: %s =======\n", &deviceID);
	k_sleep(K_MSEC(3000));
	//int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);

	//sit_led_init();
	
	//int init_ok = sit_init(&config, TX_ANT_DLY, RX_ANT_DLY);
	//set_device_state("start");
	
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
	
	uint8_t frame_sequenz = 0;
	uint8_t this_initiator_node_id  = 1;
	uint8_t responder_node_id       = 2;

    uint32_t regStatus = sit_get_device_status();
    LOG_INF("statusreg = 0x%08x",regStatus);

	int count = 0;	
	float average_meausure = 0;
	int mod = 1;
	while (1) {
		if(mod == 1){
		gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);	
		count++;

		if((count % 10) == 1){
			average_meausure =0;
		}

		if(count==1){LOG_INF("=========== Cycle number: %d =======\n", count);}
		//LOG_INF("leds> hello %d",count);
		
        regStatus = sit_get_device_status();
        if(count==1){LOG_INF("initiator> sequence(%u) starting ; statusreg = 0x%08x \n",frame_sequenz,regStatus);}
        //sit_setRxAfterTxDelay(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
        sit_set_rx_tx_delay_rx_timeout(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
        msg_header_t twr_poll = {twr_1_poll, frame_sequenz, this_initiator_node_id , responder_node_id,0};
        sit_start_poll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));
        regStatus = sit_get_device_status();

        if(count==1){LOG_INF("statusreg = 0x%08x \n",regStatus);}

        frame_sequenz++;

        msg_ss_twr_final_t rx_final_msg;
		msg_id_t msg_id = ss_twr_2_resp;
        regStatus = sit_get_device_status();
        if(sit_check_final_msg_id(msg_id, &rx_final_msg)) {
            uint64_t poll_tx_ts = get_tx_timestamp_u64();
			uint64_t resp_rx_ts = get_rx_timestamp_u64();
			
            uint64_t poll_rx_ts = rx_final_msg.poll_rx_ts;
            uint64_t resp_tx_ts = rx_final_msg.resp_tx_ts;
 
            float clockOffsetRatio = dwt_readcarrierintegrator() * 
                    (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6) ;

            uint64_t rtd_init = resp_rx_ts - poll_tx_ts;
            uint64_t rtd_resp = resp_tx_ts - poll_rx_ts;

            float tof =  ((rtd_init - rtd_resp * 
                       (1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;

            float offset_distance;		
			offset_distance = 0.0;
            float distance = tof * SPEED_OF_LIGHT + offset_distance;

			if (average_meausure != 0){
			average_meausure = (average_meausure + distance) / 2;
			}
			else{
				average_meausure = distance;
			}			
            LOG_INF("initiator -> responder Distance: %3.2lf \n", distance);			
			LOG_INF("initiator -> responder averaged distance: %3.2lf \n", average_meausure);			
		} else {
			LOG_WRN("Something is wrong\n");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
        //k_sleep(K_MSEC(RNG_DELAY_MS));
		gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

		k_sleep(K_MSEC(3000));
		}
		else{
					regStatus = sit_get_device_status();
		if(count==1){LOG_INF("sequence(%u) starting ; statusreg = 0x%08x\n",frame_sequenz,regStatus);}
		sit_receive_at(0);
        msg_header_t rx_poll_msg;
		msg_id_t msg_id = twr_1_poll;
		if(sit_check_msg_id(msg_id, &rx_poll_msg)){
			uint64_t poll_rx_ts = get_rx_timestamp_u64();
            
            uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            uint32_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
            
            msg_ss_twr_final_t msg_ss_twr_final_t = {
                    ss_twr_2_resp, (uint8_t)(rx_poll_msg.header.sequence + 1),
                    rx_poll_msg.header.dest , rx_poll_msg.header.source,(uint32_t)poll_rx_ts, resp_tx_ts,0
                };
            sit_send_at((uint8_t*)&msg_ss_twr_final_t, sizeof(msg_ss_twr_final_t), resp_tx_time);

		} else {
			LOG_WRN("Something is wrong\n");
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		}
		regStatus = sit_get_device_status();
		LOG_INF("sequence(%u) ending ; statusreg = 0x%08x\n",frame_sequenz,regStatus);
		frame_sequenz++;
		k_sleep(K_MSEC(3000));
		}
	}
}


void initialization() {
	uint8_t error = 0;

	sit_led_init();
	if(device == initiator) {
		if (sit_ble_init()) {
			LOG_ERR("Bluetooth init failed");
		}
	}

	// repeat configuration when failed
	do {
		error = sit_init();
	} while (error > 1);
	
	LOG_INF("Init Fertig ");
}