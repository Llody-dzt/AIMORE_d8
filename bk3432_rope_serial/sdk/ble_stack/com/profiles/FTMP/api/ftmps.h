/**
 ****************************************************************************************
 *
 * @file fff0s.h
 *
 * @brief Header file - FTMP Service Server Role
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef _FTMPS_H_
#define _FTMPS_H_

/**
 ****************************************************************************************
 * @addtogroup  FTMP 'Profile' Server
 * @ingroup FTMP
 * @brief FTMP 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_FTMP_SERVER)

#include "ftmps_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define FTMPS_CFG_FLAG_MANDATORY_MASK       (0xFFFFFFFF)
#define FTMP_CFG_FLAG_MTP_MASK              (0x40)

#define FTMP_CHA_CCC_MAX_LEN                2


enum
{
  FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL_LEN = 20,
  FTMP_IDX_TREADMILL_DATA_VAL_LEN = 20,
  FTMP_IDX_CROSS_TRAINER_DATA_VAL_LEN  = 20,
	FTMP_IDX_STEP_CLIMBER_DATA_VAL_LEN = 20,
  FTMP_IDX_STAIR_CLIMBER_DATA_VAL_LEN = 20,
	FTMP_IDX_ROWER_DATA_VAL_LEN = 20,
	FTMP_IDX_INDOOR_BIKE_DATA_VAL_LEN = 20,
	FTMP_IDX_TRAINING_STATUS_VAL_LEN = 20,
	FTMP_IDX_SUPPORT_SPEED_RANGE_VAL_LEN = 20,
	FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL_LEN = 20,
	FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL_LEN = 20,
	FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL_LEN = 20,
	FTMP_IDX_SUPPORT_POWER_RANGE_VAL_LEN = 20,
	FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL_LEN = 20,
	FTMP_IDX_FITNESS_MACHINE_STATUS_VAL_LEN = 20,
};


/// Battery Service Attributes Indexes
enum
{
	FTMPS_IDX_SVC,

	/// Fitness Machine Feature
	FTMP_IDX_FITNESS_MACHINE_FEATURE_CHAR, 
	FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL,           
	/// Treadmill Data
	// FTMP_IDX_TREADMILL_DATA_CHAR,
	// FTMP_IDX_TREADMILL_DATA_VAL,	
	// FTMP_IDX_TREADMILL_DATA_NTY_CFG,
	/// Cross Trainer Data
	//FTMP_IDX_CROSS_TRAINER_DATA_CHAR ,
	//FTMP_IDX_CROSS_TRAINER_DATA_VAL ,	
	//FTMP_IDX_CROSS_TRAINER_DATA_NTY_CFG,	
	/// Step Climber Data
	//FTMP_IDX_STEP_CLIMBER_DATA_CHAR,
	//FTMP_IDX_STEP_CLIMBER_DATA_VAL,	 
	//FTMP_IDX_STEP_CLIMBER_DATA_NTY_CFG,		

	///Stair Climber Data
	//FTMP_IDX_STAIR_CLIMBER_DATA_CHAR ,
	//FTMP_IDX_STAIR_CLIMBER_DATA_VAL ,	
	//FTMP_IDX_STAIR_CLIMBER_DATA_NTY_CFG ,	
	///Rower Data
	//FTMP_IDX_ROWER_DATA_CHAR ,
	//FTMP_IDX_ROWER_DATA_VAL ,	
	//FTMP_IDX_ROWER_DATA_NTY_CFG ,	
	///Indoor Bike Data
	FTMP_IDX_INDOOR_BIKE_DATA_CHAR,  
	FTMP_IDX_INDOOR_BIKE_DATA_VAL,	
	FTMP_IDX_INDOOR_BIKE_DATA_NTY_CFG,		
	///Training Status
	FTMP_IDX_TRAINING_STATUS_CHAR, 
	FTMP_IDX_TRAINING_STATUS_VAL,	
	FTMP_IDX_TRAINING_STATUS_NTY_CFG,			
	///Supported Speed Range
	FTMP_IDX_SUPPORT_SPEED_RANGE_CHAR,    
	FTMP_IDX_SUPPORT_SPEED_RANGE_VAL,  		
	///Supported Inclination Range
	FTMP_IDX_SUPPORT_INCLINATION_RANGE_CHAR,	   
	FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL,	 		
	///Supported Resistance Level Range
	FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_CHAR,	 
	FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL,			
	///Supported Heart Rate Range
	FTMP_IDX_SUPPORT_HEART_RATE_RANGE_CHAR,	   
	FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL,	 		
	///Supported Power Range
	FTMP_IDX_SUPPORT_POWER_RANGE_CHAR,		 
	FTMP_IDX_SUPPORT_POWER_RANGE_VAL,			
	///Fitness Machine Control Point
	FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_CHAR,	   
	FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL,	// 20
	FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG,
	///Fitness Machine Status
	FTMP_IDX_FITNESS_MACHINE_STATUS_CHAR,   
	FTMP_IDX_FITNESS_MACHINE_STATUS_VAL,			// 23
	FTMP_IDX_FITNESS_MACHINE_STATUS_NTF_CFG,	

	///Fitness Extension Data
	FTMP_IDX_FITNESS_EXTENSION_DATA_CHAR,   
	FTMP_IDX_FITNESS_EXTENSION_DATA_VAL,			// 26

	FTMPS_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// FTMP 'Profile' Server environment variable
struct ftmps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct ke_msg * operation;
    /// FTMP Services Start Handle
    uint16_t start_hdl;
  
    uint8_t FTMP_FITNESS_MACHINE_FEATURE_VAL[FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL_LEN];
	
	  uint8_t FTMP_TREADMILL_DATA_VAL [FTMP_IDX_TREADMILL_DATA_VAL_LEN];
	  uint8_t FTMP_TREADMILL_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
	
	  uint8_t FTMP_CROSS_TRAINER_DATA_VAL[FTMP_IDX_CROSS_TRAINER_DATA_VAL_LEN];
	  uint8_t FTMP_CROSS_TRAINER_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
	
	  uint8_t FTMP_STEP_CLIMBER_DATA_VAL [FTMP_IDX_STEP_CLIMBER_DATA_VAL_LEN];
	  uint8_t FTMP_STEP_CLIMBER_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
	
//	  uint8_t FTMP_STAIR_CLIMBER_DATA_VAL [FTMP_IDX_STAIR_CLIMBER_DATA_VAL_LEN];
//	  uint8_t FTMP_STAIR_CLIMBER_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
//		
//		uint8_t FTMP_ROWER_DATA_VAL [FTMP_IDX_ROWER_DATA_VAL_LEN];
//	  uint8_t FTMP_ROWER_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
//		
//		
	  uint8_t FTMP_INDOOR_BIKE_DATA_VAL [FTMP_IDX_INDOOR_BIKE_DATA_VAL_LEN];
	  uint8_t FTMP_INDOOR_BIKE_DATA_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
		
		uint8_t FTMP_TRAINING_STATUS_VAL[FTMP_IDX_TRAINING_STATUS_VAL_LEN];
	  uint8_t FTMP_TRAINING_STATUS_NTY_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
		
		  	
		
		uint8_t FTMP_SUPPORT_SPEED_RANGE_VAL [FTMP_IDX_SUPPORT_SPEED_RANGE_VAL_LEN];
	  
		uint8_t FTMP_SUPPORT_INCLINATION_RANGE_VAL [FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL_LEN];
	 
		uint8_t FTMP_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL [FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL_LEN];
		
		uint8_t FTMP_SUPPORT_HEART_RATE_RANGE_VAL [FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL_LEN];
		
		uint8_t FTMP_SUPPORT_POWER_RANGE_VAL [FTMP_IDX_SUPPORT_POWER_RANGE_VAL_LEN];
		
		
		uint8_t FTMP_FITNESS_MACHINE_CONTROL_POINTE_VAL[FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL_LEN];
	  uint8_t FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
		
		uint8_t FTMP_FITNESS_MACHINE_STATUS_VAL[FTMP_IDX_FITNESS_MACHINE_STATUS_VAL_LEN];
	  uint8_t FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[BLE_CONNECTION_MAX][FTMP_CHA_CCC_MAX_LEN];
		
		
    /// BASS task state
    ke_state_t state[FTMPS_IDX_MAX];
	
    /// Database features
    uint8_t features;

};



/**
 ****************************************************************************************
 * @brief Retrieve fff0 service profile interface
 *
 * @return fff0 service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* ftmps_prf_itf_get(void);

uint16_t ftmps_get_att_handle(uint16_t att_index);

uint8_t  ftmps_get_att_idx(uint16_t handle, uint16_t *att_idx);

void ftmps_notify_lvl(struct ftmps_env_tag* ftmps_env, struct ftmps_level_upd_req const *param);

#endif /* #if (BLE_FTMP_SERVER) */



#endif /*  _FTMP_H_ */



