/**
 * Copyright (c) 2019 - Frederic Mes, RTLOC
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2021 - Home Smart Mesh
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This file is part of Zephyr-DWM1001.
 *
 *   Zephyr-DWM1001 is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zephyr-DWM1001 is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Zephyr-DWM1001.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "drivers/sit.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "drivers/sit_led/sit_led.h"
#include "drivers/sit_ble/ble_device.h"
#ifdef __cplusplus
}
#endif

#define APP_NAME "SIMPLE TWR Initiator BLE EXAMPLE\n"

#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_WRN);

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 500

/* Default antenna delay  => 16436*/
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

/* Default communication configuration. */
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_EXT, /* PHY header mode. */
    (129)            /* SFD timeout (preamble length + 1 + SFD length - PAC size). 
                        Used in RX only. */           
};

uint8_t this_initiator_node_id  = 1;
uint8_t responder_node_id       = 2;

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) 
 * conversion factor.
 * 1 uus = 512 / 499.2 usec and 1 usec = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

/*rx twr_2_resp after tx twr_1_poll
 protected by responder's mp_request_at(twr_2_resp):POLL_RX_TO_RESP_TX_DLY_UUS
*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 500

#define RESP_RX_TIMEOUT_UUS 800

#define STACKSIZE 2048

void main(void) {
	printk(APP_NAME);
	printk("==================\n");

	bool init_ok = sit_init(config, TX_ANT_DLY, RX_ANT_DLY);
	// INIT LED and let them Blink one Time to see Intitalion Finshed
    dwm_led_init();
    ble_device_init();

    if(init_ok == false){
        dwm_set_led(2, 0);
    } else {
        dwm_set_led(1, 0);
    }
    
    uint32_t regStatus = sit_getRegStatus();
    LOG_INF("statusreg = 0x%08x",regStatus);
    k_sleep(K_SECONDS(2)); // Allow Logging to write out put 

    uint8_t frame_sequenz = 0;
    k_yield();

    while (1) {
        k_sleep(K_SECONDS(1));
        if (is_connected()) {
            dwm_set_led(3, 0);
        } else {
            dwm_toggle_led(3);
        }
        while (is_connected()) {
            regStatus = sit_getRegStatus();
            LOG_INF("initiator> sequence(%u) starting ; statusreg = 0x%08x",frame_sequenz,regStatus);
            sit_setRxAfterTxDelay(POLL_TX_TO_RESP_RX_DLY_UUS, RESP_RX_TIMEOUT_UUS);
            msg_header_t twr_poll = {twr_1_poll, frame_sequenz, this_initiator_node_id, responder_node_id, 0};
            sit_startPoll((uint8_t*) &twr_poll, (uint16_t)sizeof(twr_poll));
            regStatus = sit_getRegStatus();
            LOG_INF("statusreg = 0x%08x",regStatus);

            frame_sequenz++;

            msg_ss_twr_final_t rx_final_msg;
            msg_id_t msg_id = ss_twr_2_resp;
            regStatus = sit_getRegStatus();
            if(sit_checkReceivedIdFinalMsg(msg_id, rx_final_msg)) {
                uint64_t poll_tx_ts = get_tx_timestamp_u64();
                uint64_t resp_rx_ts = get_rx_timestamp_u64();
                
                uint64_t poll_rx_ts = rx_final_msg.poll_rx_ts;
                uint64_t resp_tx_ts = rx_final_msg.resp_tx_ts;
    
                float clockOffsetRatio = dwt_readcarrierintegrator() * 
                        (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_2 / 1.0e6) ;

                uint64_t rtd_init = resp_rx_ts - poll_tx_ts;
                uint64_t rtd_resp = resp_tx_ts - poll_rx_ts;

                float tof =  ((rtd_init - rtd_resp * 
                        (1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
                
                float distance = tof * SPEED_OF_LIGHT;
                printk("initiator -> responder Distance: %3.2lf \n", distance);
                if((int) distance >= 0) {
                    ble_sit_notify(distance);
                }
            } else {
                LOG_WRN("Something is wrong");
                dwt_rxreset();
            }
            k_sleep(K_MSEC(RNG_DELAY_MS));
        }
	}
}
