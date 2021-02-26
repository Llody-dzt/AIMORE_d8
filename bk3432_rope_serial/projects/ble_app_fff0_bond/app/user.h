/**
 ****************************************************************************************
 *
 * @file app_fff0.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2016.05.31
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
#ifndef APP_USER_H_
#define APP_USER_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief fff0 Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition

#include "user_config.h"
/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */


typedef struct
{
	uint8_t configflag;
	uint8_t ver[41];
	uint8_t name[21];
	uint8_t addr[6];
	uint8_t addr_len;
	uint8_t addr_type;
	uint8_t uart;
	uint8_t pair;
}TC_USER_CONFIG_S;

extern TC_USER_CONFIG_S tcuserconfig;



void user_init(void);
											 
void apptcbletxhandle(uint8_t status);
void apptcblerxhandle(uint8_t *data, uint16_t len);											 
void apptcuserconfigread(void);
void apptcuserconfig_write(void);
void apptcuserconfiginit(void);
uint8_t apptccommamdhandle(uint8_t *commad, uint16_t len, uint8_t *rsp, uint16_t *rsplen);
void apptcuartrxcompeletset(uint8_t status);
void apptcuartrxdatain(uint8_t *data, uint16_t len);
void apptcuarttxdata(uint8_t *data, uint16_t len);
void apptcconnhandle(void);
void apptcdisconnhandle(void);											 
void apptcsetblemaxmtu(uint16_t len);											 
											 
void app_clear(void);

void apptc_uart_rx_handle(void);

int app_heartbeat_timer_handler( ke_msg_id_t const msgid,
									 void *param,
									 ke_task_id_t const dest_id,
									 ke_task_id_t const src_id );

int app_check_drive_timer_handler( ke_msg_id_t const msgid,
                                     void *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id );

int app_inquire_timer_handler( ke_msg_id_t const msgid,
                                     void *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id );

void apptc_data_decode(uint8_t *data, uint8_t len);

void apptc_cmd_decode(uint8_t cmd, uint8_t *data, uint8_t len);

void apptc_cmd_encode(uint8_t cmd, uint8_t *data, uint8_t len);			
																		 
uint8_t user_check_drive(uint8_t *data, uint8_t len);

#endif
