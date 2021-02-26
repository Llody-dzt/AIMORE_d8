/**
 ****************************************************************************************
 *
 * @file ftmps.c
 *
 * @brief ftmp Server Implementation.
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FTMP_SERVER)
#include "attm.h"
#include "ftmps.h"
#include "ftmps_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "ke_mem.h"

#include "uart.h"

#include "attm_util128.h"


/*
 * FTMP ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
 
enum
{		
		ATT_USER_SERVER_FCC0 						= ATT_UUID_16(0x2c10),
};
 
#define GATT_UUID_FITNESS_EXTENSION_DATA                  {0x59,0x14,0xfb,0x69,0x92,0x52,0x55,0xa3,0xe8,0x4c,0x11,0xc4,0x10,0x2c,0x8d,0xd1}
 
/// Full FTMP Database Description - Used to add attributes into the database
const struct attm_desc ftmp_att_db[FTMPS_IDX_NB] =
{
    // FTMP Service Declaration
  	[FTMPS_IDX_SVC]            =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

	[FTMP_IDX_FITNESS_MACHINE_FEATURE_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	[FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL]   =   {ATT_CHAR_FITNESS_MACHINE_FEATURE,PERM(RD, ENABLE), PERM(RI, ENABLE), FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL_LEN *sizeof(uint8_t)},
	
	// [FTMP_IDX_TREADMILL_DATA_CHAR]           =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  // 	[FTMP_IDX_TREADMILL_DATA_VAL]            =   {ATT_CHAR_TREADMILL_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_TREADMILL_DATA_VAL_LEN * sizeof(uint8_t)},
	// [FTMP_IDX_TREADMILL_DATA_NTY_CFG]        = 	 {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

	//[FTMP_IDX_CROSS_TRAINER_DATA_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	//[FTMP_IDX_CROSS_TRAINER_DATA_VAL]        =   {ATT_CHAR_CROSS_TRAINER_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_CROSS_TRAINER_DATA_VAL_LEN * sizeof(uint8_t)},
	//[FTMP_IDX_CROSS_TRAINER_DATA_NTY_CFG] 	 =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	//[FTMP_IDX_STEP_CLIMBER_DATA_CHAR]        =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	//[FTMP_IDX_STEP_CLIMBER_DATA_VAL]         =   {ATT_CHAR_STEP_CLIMBER_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_STEP_CLIMBER_DATA_VAL_LEN * sizeof(uint8_t)},
	//[FTMP_IDX_STEP_CLIMBER_DATA_NTY_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
// [FTMP_IDX_STAIR_CLIMBER_DATA_CHAR]       =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
//  [FTMP_IDX_STAIR_CLIMBER_DATA_VAL]        =   {ATT_CHAR_STAIR_CLIMBER_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_STAIR_CLIMBER_DATA_VAL_LEN * sizeof(uint8_t)},
//	[FTMP_IDX_STAIR_CLIMBER_DATA_NTY_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
//	
//	[FTMP_IDX_ROWER_DATA_CHAR]               =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
//  [FTMP_IDX_ROWER_DATA_VAL]                =   {ATT_CHAR_ROWER_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_ROWER_DATA_VAL_LEN * sizeof(uint8_t)},
//	[FTMP_IDX_ROWER_DATA_NTY_CFG]            = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
//	
	[FTMP_IDX_INDOOR_BIKE_DATA_CHAR]         =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  [FTMP_IDX_INDOOR_BIKE_DATA_VAL]          =   {ATT_CHAR_INDOOR_BIKE_DATA, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_INDOOR_BIKE_DATA_VAL_LEN * sizeof(uint8_t)},
	[FTMP_IDX_INDOOR_BIKE_DATA_NTY_CFG]      = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	[FTMP_IDX_TRAINING_STATUS_CHAR]          =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  [FTMP_IDX_TRAINING_STATUS_VAL]           =   {ATT_CHAR_TRAINING_STATUS, PERM(NTF, ENABLE)|PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_TRAINING_STATUS_VAL_LEN * sizeof(uint8_t)},
	[FTMP_IDX_TRAINING_STATUS_NTY_CFG]       = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	[FTMP_IDX_SUPPORT_SPEED_RANGE_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  [FTMP_IDX_SUPPORT_SPEED_RANGE_VAL]       =   {ATT_CHAR_SUPPORT_SPEED_RANGE, PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_SUPPORT_SPEED_RANGE_VAL_LEN * sizeof(uint8_t)},
	
	// [FTMP_IDX_SUPPORT_INCLINATION_RANGE_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  // [FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL]   =   {ATT_CHAR_SUPPORT_INCLINATION_RANGE, PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL_LEN * sizeof(uint8_t)},
	
	// [FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  // [FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL]   =   {ATT_CHAR_SUPPORT_RESISTANCE_LEVEL_RANGE, PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL_LEN * sizeof(uint8_t)},
	
	[FTMP_IDX_SUPPORT_HEART_RATE_RANGE_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  [FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL]   =   {ATT_CHAR_SUPPORT_HEART_RATE_RANGE, PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL_LEN * sizeof(uint8_t)},
	
	[FTMP_IDX_SUPPORT_POWER_RANGE_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  [FTMP_IDX_SUPPORT_POWER_RANGE_VAL]       =   {ATT_CHAR_SUPPORT_POWER_RANGE, PERM(RD, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_SUPPORT_POWER_RANGE_VAL_LEN * sizeof(uint8_t)},
	
	[FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	[FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL]   =   {ATT_CHAR_FITNESS_MACHINE_CONTROL_POINTE, PERM(WRITE_REQ, ENABLE)|PERM(IND, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL_LEN * sizeof(uint8_t)},
	[FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	[FTMP_IDX_FITNESS_MACHINE_STATUS_CHAR]    =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	[FTMP_IDX_FITNESS_MACHINE_STATUS_VAL]     =   {ATT_CHAR_FITNESS_MACHINE_STATUS, PERM(NTF, ENABLE) , PERM(RI, ENABLE), FTMP_IDX_FITNESS_MACHINE_STATUS_VAL_LEN * sizeof(uint8_t)},
	[FTMP_IDX_FITNESS_MACHINE_STATUS_NTF_CFG] =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	[FTMP_IDX_FITNESS_EXTENSION_DATA_CHAR]    =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
  	[FTMP_IDX_FITNESS_EXTENSION_DATA_VAL]     =   {ATT_USER_SERVER_FCC0, PERM(WRITE_REQ, ENABLE) , PERM(UUID_LEN, UUID_128)|PERM(RI, ENABLE)|ATT_UUID_128_LEN, FTMP_IDX_FITNESS_MACHINE_STATUS_VAL_LEN * sizeof(uint8_t)},
	
};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t ftmps_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ftmps_db_cfg* params)
{
    uint16_t shdl;
    struct ftmps_env_tag* ftmps_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ftmps_env = (struct ftmps_env_tag* ) ke_malloc(sizeof(struct ftmps_env_tag), KE_MEM_ATT_DB);
    memset(ftmps_env, 0 , sizeof(struct ftmps_env_tag));

    // Service content flag
    uint32_t cfg_flag = FTMPS_CFG_FLAG_MANDATORY_MASK;

    // Save database configuration
    ftmps_env->features |= (params->features);
   
   
    shdl = *start_hdl;

    //Create FTMP in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_util_svc_create_db128(&(shdl), ATT_SVC_FITNESS_MACHINE, (uint8_t *)&cfg_flag,
            FTMPS_IDX_NB, NULL, env->task, &ftmp_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
				


    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ftmps_env;
        *start_hdl = shdl;
        ftmps_env->start_hdl = *start_hdl;
        ftmps_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ftmps_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_FTMPS;
        env->desc.idx_max           = FTMPS_IDX_MAX;
        env->desc.state             = ftmps_env->state;
        env->desc.default_handler   = &ftmps_default_handler;

        // service is ready, go into an Idle state
        ke_state_set(env->task, FTMPS_IDLE);
    }
    else if(ftmps_env != NULL)
    {
        ke_free(ftmps_env);
    }
     
    return (status);
}


static void ftmps_destroy(struct prf_task_env* env)
{
    struct ftmps_env_tag* ftmps_env = (struct ftmps_env_tag*) env->env;

    // clear on-going operation
    if(ftmps_env->operation != NULL)
    {
        ke_free(ftmps_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(ftmps_env);
}

static void ftmps_create(struct prf_task_env* env, uint8_t conidx)
{
	struct ftmps_env_tag* ftmps_env = (struct ftmps_env_tag*) env->env;
	ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

	// force notification config to zero when peer device is connected

	ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][0] = 0x00;		   
	ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_STAIR_CLIMBER_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_STAIR_CLIMBER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_ROWER_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_ROWER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_INDOOR_BIKE_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_INDOOR_BIKE_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][1] = 0x00;
}


static void ftmps_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
	struct ftmps_env_tag* ftmps_env = (struct ftmps_env_tag*) env->env;

	ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
	// force notification config to zero when peer device is disconnected
	ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][0] = 0x00;		   
	ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_STAIR_CLIMBER_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_STAIR_CLIMBER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_ROWER_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_ROWER_DATA_NTY_CFG[conidx][1] = 0x00;

	//ftmps_env->FTMP_INDOOR_BIKE_DATA_NTY_CFG[conidx][0] = 0x00;
	//ftmps_env->FTMP_INDOOR_BIKE_DATA_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][1] = 0x00;

	ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][0] = 0x00;
	ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][1] = 0x00;
}


void ftmps_notify_lvl(struct ftmps_env_tag* ftmps_env, struct ftmps_level_upd_req const *param)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *lvl = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(ftmps_env->prf_env),0),
            gattc_send_evt_cmd, sizeof(uint8_t)* (param->length));

	if(param->uuid_indx == FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL)
	{
		lvl->operation = GATTC_INDICATE;
	}
	else
	{
    	// Fill in the parameter structure
    	lvl->operation = GATTC_NOTIFY;
	}
    lvl->handle = ftmps_get_att_handle(param->uuid_indx);
    // pack measured value in database
    lvl->length = param->length;
  	//fff1_lvl->value[0] = ftmps_env->fff1_lvl[0];
	  memcpy(&lvl->value[0],&param->level[0],param->length);
    // send notification to peer device
    ke_msg_send(lvl);
}



/// BASS Task interface required by profile manager
const struct prf_task_cbs ftmps_itf =
{
        (prf_init_fnct) ftmps_init,
        ftmps_destroy,
        ftmps_create,
        ftmps_cleanup,
};


const struct prf_task_cbs* ftmps_prf_itf_get(void)
{
   return &ftmps_itf;
}


uint16_t ftmps_get_att_handle(uint16_t att_idx)
{
		
    struct ftmps_env_tag *ftmps_env = PRF_ENV_GET(FTMPS, ftmps);
    uint16_t handle = ATT_INVALID_HDL;
   
    handle = ftmps_env->start_hdl;

    // increment index according to expected index
    if(att_idx < FTMPS_IDX_NB)
    {
        handle += att_idx;
    }     
    else
    {
        handle = ATT_INVALID_HDL;
    }

    return handle;
}

uint8_t ftmps_get_att_idx(uint16_t handle, uint16_t *att_idx)
{
    struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);
    uint16_t start_handle = ftmps_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle < (start_handle + FTMPS_IDX_NB))
    {
        *att_idx = handle -start_handle;
        status = GAP_ERR_NO_ERROR;
        
    }
    
    return (status);
}


#endif // (BLE_ftmp_SERVER)


 
