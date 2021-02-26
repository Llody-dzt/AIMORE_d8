/**
****************************************************************************************
*
* @file rf_xvras.c
*
* @brief Atlas radio initialization and specific functions
*
* Copyright (C) Beken 2009-2015
*
* $Rev: $
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup RF_XVR
* @ingroup RF
* @brief Radio Driver
*
* This is the driver block for radio
* @{
****************************************************************************************
*/

/**
 *****************************************************************************************
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <string.h>        // for memcpy
#include "co_utils.h"      // common utility definition
#include "co_math.h"       // common math functions
#include "co_endian.h"     // endian definitions
#include "rf.h"            // RF interface
#include "em_map.h"        // RF area mapping

#include "rwip.h"          // for RF API structure definition
#include "reg_blecore.h"   // ble core registers
#include "reg_ble_em_cs.h" // control structure definitions

#include "BK3432_reg.h"           // Platform register
#include "icu.h"
#include "rwip_config.h" // stack configuration




#define RPL_POWER_MAX               0x0f


void initial_xver_BK3432_openLoop(void);
void CLK32K_AutoCali_init(void);


/**
 *****************************************************************************************
 * @brief Init RF sequence after reset.
 *****************************************************************************************
 */

static void rf_reset(void)
{
}

/**
 ****************************************************************************************
 * @brief ISR to be called in BLE ISR routine when RF Interrupt occurs.
 *****************************************************************************************
 */


/**
 *****************************************************************************************
 * @brief Get TX power in dBm from the index in the control structure
 *
 * @param[in] txpwr_idx  Index of the TX power in the control structure
 * @param[in] modulation Modulation: 1 or 2 or 3 MBPS
 *
 * @return The TX power in dBm
 *
 *****************************************************************************************
 */

static uint8_t rf_txpwr_dbm_get(uint8_t txpwr_idx, uint8_t modulation)
{
	// Power table should be provided
	return(0);
}

/**
 *****************************************************************************************
 * @brief Sleep function for  RF.
 *****************************************************************************************
 */

static void rf_sleep(void)
{
#if defined(CFG_BLE)
	ble_deepslcntl_set(ble_deepslcntl_get() |
	                   BLE_DEEP_SLEEP_ON_BIT |    // RW BLE Core sleep
	                   BLE_RADIO_SLEEP_EN_BIT |   // Radio sleep
	                   BLE_OSC_SLEEP_EN_BIT);     // Oscillator sleep
#endif // CFG_BLE
}


/**
 *****************************************************************************************
 * @brief Convert RSSI to dBm
 *
 * @param[in] rssi_reg RSSI read from the HW registers
 *
 * @return The converted RSSI
 *
 *****************************************************************************************
 */

int8_t rf_rssi_convert (uint8_t rssi_reg)
{
	uint8_t RssidBm = 0;

	RssidBm = ((rssi_reg) >> 1) - 118;

	return(RssidBm);
}


static uint32_t rf_rpl_reg_rd (uint16_t addr)
{
	uint32_t ret;

	ret = REG_PL_RD(addr);

	return ret;
}

static void rf_rpl_reg_wr (uint16_t addr, uint32_t value)
{
	REG_PL_WR(addr, value);
}
/**
 ****************************************************************************************
 * RADIO FUNCTION INTERFACE
 ****************************************************************************************
 **/

static void rf_force_agc_enable(bool en)
{

	ble_forceagc_en_setf(en);

}
void rf_init(struct rwip_rf_api *api)
{
	// Initialize the RF driver API structure
	api->reg_rd = rf_rpl_reg_rd;
	api->reg_wr = rf_rpl_reg_wr;
	api->txpwr_dbm_get = rf_txpwr_dbm_get;
	api->txpwr_max = RPL_POWER_MAX;
	api->sleep = rf_sleep;
	api->reset = rf_reset;

#if defined(CFG_BLE)
	api->force_agc_enable = rf_force_agc_enable;
#endif //CFG_BLE

	api->rssi_convert = rf_rssi_convert;

	initial_xver_BK3432_openLoop();
}

void Delay_us(int num)
{
	int x, y;
	for(y = 0; y < num; y ++ )
	{
		for(x = 0; x < 10; x++);
	}
}

void Delay(int num)
{
	int x, y;
	for(y = 0; y < num; y ++ )
	{
		for(x = 0; x < 50; x++);
	}
}

void Delay_ms(int num) //sync from svn revision 18
{
	int x, y;
	for(y = 0; y < num; y ++ )
	{
		for(x = 0; x < 3260; x++);
	}

}


uint32_t XVR_ANALOG_REG_BAK[9] = {0};

void kmod_calibration(void) 
{
    uint32_t value;
    uint32_t value_kcal_result;
    
    XVR_REG24  |= (0x1 << 20);
    XVR_REG24  |= (0xf << 7);
    XVR_REG24  &= ~(0x1 << 17);
    XVR_REG24  &= ~(0x7f);
    XVR_REG25 |= (1<<12);
    Delay_ms(10);
    XVR_REG25 |= (1<<13);

    Delay_ms(50);

    XVR_ANALOG_REG_BAK[3] &= ~(0x1 << 6);
    XVR_REG03 = XVR_ANALOG_REG_BAK[3];
    Delay_ms(10);

    XVR_ANALOG_REG_BAK[3] |= (0x1 << 7);
    XVR_REG03 = XVR_ANALOG_REG_BAK[3];
    Delay_ms(10);
    XVR_REG25 |= (1<<16);

    Delay_ms(100);
    value = XVR_REG12;

    value = ((value >> 16) & 0x1fff);


    value_kcal_result =  ((256*250/value)&0x1ff) ;  //2M :((256*250/value)&0x1ff); 1M:((256*125/value)&0x1ff)
    if(value_kcal_result>0x100)
        value_kcal_result=0x100;

    //	uart_printf_test("value = 0x%x,value_kcal_result = 0x%x\r\n",value,value_kcal_result);
    XVR_REG30 &= (~(0x1ff<<8));
    XVR_REG30 |= (value_kcal_result<<8);
    Delay_ms(100);
    XVR_REG25 &= (~(1<<12));
    XVR_REG25 &= (~(1<<13));

    XVR_ANALOG_REG_BAK[3] &= ~(0x1 << 7);
    XVR_REG03 = XVR_ANALOG_REG_BAK[3];
    Delay_ms(10); 

    XVR_ANALOG_REG_BAK[3] |= (0x1 << 6); 
    XVR_REG03 = XVR_ANALOG_REG_BAK[3];

    XVR_REG25 &= (~(1<<16));

    XVR_REG24 |= (0x1 << 17);
    XVR_REG24  &= ~(0x1 << 20);

    XVR_REG30 |= (0x01 << 0);
    Delay_ms(100);
    Delay_ms(200);

    XVR_ANALOG_REG_BAK[0x4] = XVR_REG24;
    XVR_ANALOG_REG_BAK[0x5] = XVR_REG25;
    XVR_ANALOG_REG_BAK[0x6] = XVR_REG20;   
    XVR_ANALOG_REG_BAK[0x7] = XVR_REG23; 
    XVR_ANALOG_REG_BAK[0x8] = XVR_REG3B; 
}



void CLK32K_AutoCali_init(void);


void initial_xver_BK3432_openLoop(void)
{ 
    //Delay_ms(10);
    XVR_REG00 = 0XC4B03210;//0XC4B03210; // REG_0     
    XVR_REG01 = 0X8295C200;//0X8295C300; // REG_1     
    XVR_REG02 = 0X2F22A000;//0X2F22A000; // REG_2     
    XVR_REG03 = 0X2F62A262;//0X2F62A462; // REG_3 
    XVR_REG04 = 0X5F915ECB;//0X5F9156CB; // REG_4     
    XVR_REG05 = 0X48205213;//0X48205211; // REG_5     
    XVR_REG06 = 0XE4FA0A00;//0X847A8A00; // REG_6     
    XVR_REG07 = 0XAA003FC4;//0XAA007DC4;//0XAA003DC4; // REG_7     
    XVR_REG08 = 0X07C08005;//0X7C1C00d ; // REG_8    
    
#if (HZ32000)	
	XVR_REG09 = 0X70203C48; // REG_9
#else	
    XVR_REG09 = 0X74203C48; // REG_9
#endif

#if (LDO_MODE)
    XVR_REG0A = 0X9C03786B;//0X9C275843;
#else
    XVR_REG0A = 0X9C03786F;//0X9C275847; // REG_A     
#endif
    XVR_REG0B = 0X0E67BF23;//0X0FD93F23; // REG_B     
    XVR_REG0C = 0X8000D008; // REG_C     
    XVR_REG0D = 0X00000000; // REG_D     
    XVR_REG0E = 0X00000000; // REG_E     
    XVR_REG0F = 0X00000000; // REG_F 

    
    XVR_ANALOG_REG_BAK[0x0] = 0XC4B03210; // REG_0  
    XVR_ANALOG_REG_BAK[0x1] = 0X8295C200; // REG_1  
    XVR_ANALOG_REG_BAK[0x2] = 0X2F22A000; // REG_2  
    XVR_ANALOG_REG_BAK[0x3] = 0X2F62A262; // REG_3  

    													 
    XVR_REG10 = 0x00083435; // REG_10   
    XVR_REG11 = 0x16810020; // REG_11   
    XVR_REG12 = 0x00003C00; // REG_12   
    XVR_REG13 = 0x00000000; // REG_13   
    XVR_REG14 = 0x00080000; // REG_14   
    XVR_REG15 = 0x00000000; // REG_15   
    XVR_REG16 = 0x00000000; // REG_16   
    XVR_REG17 = 0x00000000; // REG_17   
    XVR_REG18 = 0x000001FF; // REG_18   
    XVR_REG19 = 0x00000000; // REG_19   
    XVR_REG1A = 0x00000000; // REG_1A   
    XVR_REG1B = 0x00000000; // REG_1B   
    XVR_REG1C = 0x00000000; // REG_1C   
    XVR_REG1D = 0x00000000; // REG_1D   
    XVR_REG1E = 0X00000000; // REG_1E   
    XVR_REG1F = 0X00000000; // REG_1F   


   
   //reg0x2a = 0x120840d;  reg0x2c = 0x006a404d ; 
    XVR_REG20 = 0x8E89BED6;// REG_20   
    XVR_REG21 = 0x96000000;//0x96000000;// REG_21   
    XVR_REG22 = 0x78000000;// REG_22   
    XVR_REG23 = 0xA0000000;// REG_23   
    XVR_REG24 = 0x001E0082;//0x00080082;// REG_24   
    XVR_REG25 = 0X00200000;// REG_25   
    XVR_REG26 = 0x10840505;// REG_26   
    XVR_REG27 = 0x0008C900;// REG_27   
    XVR_REG28 = 0x01011010;// REG_28   
    XVR_REG29 = 0x3C104E00;// REG_29   
    XVR_REG2A = 0x0e10384d;//0x0e10404d;//0x0e103D68;// REG_2A   
    XVR_REG2B = 0x00000408;// REG_2B   
    XVR_REG2C = 0x006A404d;// REG_2C   //0x006a404d
    XVR_REG2D = 0x082CC446;// REG_2D 0x082CC444 
    XVR_REG2E = 0x00000000;// REG_2E   
    XVR_REG2F = 0X00000000;// REG_2F   
    XVR_REG30 = 0x10010001;// REG_30   
    XVR_REG31 = 0X00000000;// REG_31   
    XVR_REG32 = 0X00000000;// REG_32   
    XVR_REG33 = 0X00000000;// REG_33   
    XVR_REG34 = 0X00000000;// REG_34   
    XVR_REG35 = 0X00000000;// REG_35   
    XVR_REG36 = 0X00000000;// REG_36   
    XVR_REG37 = 0X00000000;// REG_37   
    XVR_REG38 = 0X00000000;// REG_38   
    XVR_REG39 = 0X00000000;// REG_39   
    XVR_REG3A = 0x00128000;// REG_3A   
    XVR_REG3B = 0x36341048;// REG_3B 0x22341048  
    XVR_REG3C = 0x01FF1c00;// REG_3C   
    XVR_REG3D = 0x00000000;// REG_3D   
    XVR_REG3E = 0X00000000;// REG_3E   
    XVR_REG3F = 0X00000000;// REG_3F   
    XVR_REG40 = 0x01000000;// REG_40   
    XVR_REG41 = 0x07050402;// REG_41   
    XVR_REG42 = 0x120F0C0A;// REG_42   
    XVR_REG43 = 0x221E1A16;// REG_43   
    XVR_REG44 = 0x35302B26;// REG_44   
    XVR_REG45 = 0x4B45403A;// REG_45   
    XVR_REG46 = 0x635D5751;// REG_46   
    XVR_REG47 = 0x7C767069;// REG_47   
    XVR_REG48 = 0x968F8983;// REG_48   
    XVR_REG49 = 0xAEA8A29C;// REG_49   
    XVR_REG4A = 0xC5BFBAB4;// REG_4A   
    XVR_REG4B = 0xD9D4CFCA;// REG_4B   
    XVR_REG4C = 0xE9E5E1DD;// REG_4C   
    XVR_REG4D = 0xF5F3F0ED;// REG_4D   
    XVR_REG4E = 0xFDFBFAF8;// REG_4E   
    XVR_REG4F = 0xFFFFFFFE;// REG_4F  

    kmod_calibration();	

   
#if (HZ32000)
    //CLK32K_AutoCali_init();
#endif
    
} 


void CLK32K_AutoCali_init(void)
{
	XVR_REG0C = 0x80005008;
    XVR_REG0C = 0x80009008;

    Delay_ms(20);

}


//配置单载波发射
//freq:频点设置，双频点(2-80)
//power:功率等级(0x1-0xf)
void bk3432_singleWaveCfg(uint8_t freq, uint8_t power_level)
{
	uint32_t val = 0;

	val |= freq;
	val |= (power_level<< 7);
	XVR_REG04 = 0X7F9156CB;//(0x1 << 29);
	XVR_REG24 &= ~(0x1 << 20);
	XVR_REG24 &= ~(0xf << 7);
	XVR_REG24 = val;
	XVR_REG25 &= 0xFFF00000;
	XVR_REG25 |= (0x1<<12) |(0x1<<13);

	while(1);
}

void bk3432_tx_en(uint8_t freq)
{
	XVR_REG24 = freq;
	XVR_REG25 &= 0xFFF00000;
	XVR_REG25 |= (0x1<<12) |(0x1<<13);
    while(1);
}



