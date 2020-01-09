/***************************************************************************//**
 * @file app.c
 * @brief Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

#include "app.h"
#include "dump.h"

#include "dump.h"
#include "pool.h"

#define PL(X) printf("%s: object_size: %d, free: %d, buffer: %p, size: %d, head: %p\n",#X,X.object_size,X.free,X.buffer,X.size,X.head)
#define P(X) if(X.size)printf("%s: free: %d\n",#X,X.free)

int advertising = 0;
struct bg_pool_alloc backup[bg_pool_object_max];
uint8 conn;
uint8 state = 0;
uint16 response_char;

void start_advertising(void) {
	if(advertising) return;

	/* Set advertising parameters. 100ms advertisement interval.
	 * The first parameter is advertising set handle
	 * The next two parameters are minimum and maximum advertising interval, both in
	 * units of (milliseconds * 1.6).
	 * The last two parameters are duration and maxevents left as default. */
	gecko_cmd_le_gap_bt5_set_adv_data(1,0,17,(uint8*)"\0x11\0xff\0xff\0xffHello, World!");
	gecko_cmd_le_gap_bt5_set_adv_data(1,1,17,(uint8*)"\0x11\0xff\0xff\0xffHello, World!");
	gecko_cmd_le_gap_set_advertise_timing(1, 160, 160, 0, 0);
	gecko_cmd_le_gap_start_advertising(1, le_gap_user_data, le_gap_scannable_non_connectable);

	gecko_cmd_le_gap_set_advertise_timing(0, 160, 160, 0, 0);

	/* Start general advertising and enable connections. */
	if(!gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable)) advertising = 1;
}

/* Main application */
void appMain(gecko_configuration_t *pconfig)
{
#if DISABLE_SLEEP > 0
	pconfig->sleep.flags = 0;
#endif

	/* Initialize debug prints. Note: debug prints are off by default. See DEBUG_LEVEL in app.h */
	initLog();

	/* Initialize stack */
	gecko_init(pconfig);

	init_usart_ldma(&bg_pool_pools[bg_pool_object_bgbuf].free);

	while (1) {
		/* Event pointer for handling events */
		struct gecko_cmd_packet* evt;

		evt = gecko_peek_event();
		if(memcmp(backup,bg_pool_pools,sizeof(backup))) {
			if(backup[bg_pool_object_bgbuf].free != bg_pool_pools[bg_pool_object_bgbuf].free){
				P(bg_pool_pools[bg_pool_object_hci]);
				P(bg_pool_pools[bg_pool_object_bgbuf]);
				P(bg_pool_pools[bg_pool_object_acl]);
				P(bg_pool_pools[bg_pool_object_bgbuf_small]);
				P(bg_pool_pools[bg_pool_object_hci_small]);
				P(bg_pool_pools[bg_pool_object_ll_connection]);
				P(bg_pool_pools[bg_pool_object_ll_adv_set]);
				P(bg_pool_pools[bg_pool_object_gap_context]);
				P(bg_pool_pools[bg_pool_object_ll_validator]);
				P(bg_pool_pools[bg_pool_object_mesh_prov_db]);
				P(bg_pool_pools[bg_pool_object_bg_message]);
				P(bg_pool_pools[bg_pool_object_endpoint]);
				P(bg_pool_pools[bg_pool_object_soft_timer_message]);
				P(bg_pool_pools[bg_pool_object_dfu]);
				P(bg_pool_pools[bg_pool_object_foundation_cmd]);
				P(bg_pool_pools[bg_pool_object_ll_adv_sync]);
				P(bg_pool_pools[bg_pool_object_ll_scan_sync]);
				P(bg_pool_pools[bg_pool_object_gap_periodic_adv]);
				P(bg_pool_pools[bg_pool_object_gap_periodic_sync]);
				P(bg_pool_pools[bg_pool_object_l2cap_coc_channel]);
				P(bg_pool_pools[bg_pool_object_mesh_prov_db_appkey]);
			}
			memcpy(backup,bg_pool_pools,sizeof(backup));
		}
		switch(state) {
		case 1:
			if (0 == gecko_cmd_gatt_server_send_user_write_response(conn,gattdb_write,0)->result) state++;
			break;
		}
		if(evt)dump_event(evt);
		/* Handle events */
		if (evt) switch (BGLIB_MSG_ID(evt->header)) {
		case gecko_evt_system_boot_id:
			gecko_cmd_hardware_set_soft_timer(1<<15,2,0); // heartbeat
			gecko_cmd_system_get_bt_address();
			gecko_cmd_sm_set_bondable_mode(1);
			gecko_cmd_sm_configure(1,sm_io_capability_displayonly);
			start_advertising();
			P(bg_pool_pools[bg_pool_object_bgbuf]);
			break;

			break;

		case gecko_evt_sm_passkey_display_id:
			printf("passkey: %06ld",evt->data.evt_sm_passkey_display.passkey);
			break;

		case gecko_evt_le_connection_opened_id:
			conn = evt->data.evt_le_connection_opened.connection;
			gecko_cmd_le_gap_stop_advertising(1);
			advertising = 0;
			break;

		case gecko_evt_hardware_soft_timer_id:
#define ED evt->data.evt_hardware_soft_timer
			switch(ED.handle) {
			case 1:
				gecko_cmd_le_connection_close(conn);
				break;
			case 2: // heart beat
				gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name,0,10,(uint8*)"Hello, World!");
				for (struct bg_pool_alloc_data *ptr = bg_pool_pools[bg_pool_object_bgbuf].head; ptr; ptr = ptr->next) {
					printf("ptr: %p\n",ptr);
				}
			}
			break;
#undef ED

			case gecko_evt_le_connection_closed_id:
				gecko_cmd_hardware_set_soft_timer(0,1,1); // connection already closed
				start_advertising();
				break;

			case gecko_evt_gatt_server_user_write_request_id: /*********************************************** gatt_server_user_write_request **/
#define ED evt->data.evt_gatt_server_user_write_request
				switch(ED.characteristic) {
				case gattdb_write:
					state = 1;
					gecko_cmd_hardware_set_soft_timer(1<<15,1,1);
				}
				break;
#undef ED

				default:
					break;
		}
	}
}
