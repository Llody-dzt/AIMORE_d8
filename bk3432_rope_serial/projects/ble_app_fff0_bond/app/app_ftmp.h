/**
 ****************************************************************************************
 *
 * @file app_ftmp.c
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
#ifndef APP_FTMP_H_
#define APP_FTMP_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief ftmp Application Module entry point
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
#include "ftmps_task.h"

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// ftmps Application Module Environment Structure
struct app_ftmp_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current Battery Level
    uint8_t fff1_lvl;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */


/// ftmps Application environment
extern struct app_ftmp_env_tag app_ftmp_env;

/// Table of message handlers
extern const struct ke_state_handler app_ftmp_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * ftmps Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize ftmps Application Module
 ****************************************************************************************
 */
void app_ftmp_init(void);
/**
 ****************************************************************************************
 * @brief Add a ftmp Service instance in the DB
 ****************************************************************************************
 */
void app_ftmp_add_ftmps(void);
/**
 ****************************************************************************************
 * @brief Enable the ftmp Service
 ****************************************************************************************
 */
void app_ftmp_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_ftmp_send_lvl(uint8_t batt_lvl);





void SendUartDataToApp(uint8_t* data,uint8_t length, uint16_t uuidIndex);// ���͵�APP�ĺ���

void SendAppMachineControlPointToUart(uint8_t opCode,control_point_target_union *param); // ���Ϳ���������ڵĺ���

void SendAppExtensionDataToUart(huawei_fitness_extension_data_t *extensionData); // ���ͻ�Ϊ��չ�ֶε����ڵĺ���


void my_user_init(void);

void user_gpio_int_cb(void);

int app_key_timer_handler(ke_msg_id_t const msgid,
                                          void *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id);


#endif // APP_BATT_H_
