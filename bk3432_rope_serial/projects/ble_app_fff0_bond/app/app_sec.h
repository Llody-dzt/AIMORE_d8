/**
 ****************************************************************************************
 *
 * @file app_sec.h
 *
 * @brief Application Security Entry Point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_SEC
 * @{
 ****************************************************************************************
 */

#ifndef APP_SEC_H_
#define APP_SEC_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"


#include <stdint.h>          // Standard Integer Definition

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * STRUCTURES DEFINITIONS
 ****************************************************************************************
 */

struct app_sec_env_tag
{
	// Bond status
	bool bonded;
	bool peer_pairing;
	bool peer_encrypt;
	bool bond_lost;
	bool pairing_fail;
};

typedef struct
{
	uint8_t bond;
	struct gapc_ltk ltk;  //27
	struct gap_bdaddr bdaddr;  //7
	struct gapc_irk irk;//23

} APP_SEC_DATA_T, *APP_SEC_DATA_P;


struct app_sec_tag
{
	uint8_t bond;
	struct gapc_ltk ltk;
	struct gap_bdaddr bdaddr;
	struct gapc_irk irk;
	uint16_t no_use;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// Application Security Environment
extern struct app_sec_env_tag app_sec_env;

/// Table of message handlers
extern const struct ke_state_handler app_sec_table_handler;

/*
 * GLOBAL FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the Application Security Module
 ****************************************************************************************
 */
void app_sec_init(void);


/**
 ****************************************************************************************
 * @brief Remove all bond data stored in NVDS
 ****************************************************************************************
 */
void app_sec_remove_bond(void);


/**
 ****************************************************************************************
 * @brief Get bond status
 ****************************************************************************************
 */
bool app_sec_get_bond_status(void);


/**
 ****************************************************************************************
 * @brief Send a security request to the peer device. This function is used to require the
 * central to start the encryption with a LTK that would have shared during a previous
 * bond procedure.
 *
 * @param[in]   - conidx: Connection Index
 ****************************************************************************************
 */
void app_sec_send_security_req(uint8_t conidx);


#endif // APP_SEC_H_

/// @} APP_SEC

