/**
****************************************************************************************
*
* @file icu.c
*
* @brief icu initialization and specific functions
*
* Copyright (C) Beken 2009-2016
*
* $Rev: $
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup ICU
* @ingroup ICU
* @brief ICU
*
* This is the driver block for ICU
* @{
****************************************************************************************
*/


#include <stddef.h>     // standard definition
#include "rwip_config.h"
#include "user_config.h"
#include "BK3432_reg.h"
#include "gpio.h"
#include "icu.h"      // timer definition
#include "rf.h"




static volatile uint8_t reduce_voltage_set=0;
static volatile uint8_t default_sleep_mode = 0;  //0:Ωµ—π–›√ﬂ   1:∆’Õ®idel
extern uint8_t system_sleep_flag;

void system_sleep_init(void)
{
#if SYSTEM_SLEEP
	system_sleep_flag = 0x1;
#else
	system_sleep_flag = 0x0;
#endif
}

void icu_init(void)
{
#if DEBUG_HW
	REG_AHB0_ICU_LPO_CLK_ON |= (0x01 << 2);
#endif

#if LDO_MODE
	REG_AHB0_ICU_ANALOG1_PWD |= ((1<<12) | (1<<28));
#endif
	REG_AHB0_ICU_FLASH &= ~(0xff << 16);
	REG_AHB0_ICU_FLASH |= (0x15 << 16);

	REG_AHB0_ICU_CPU_STATUS  = 0x551;


	REG_AHB0_ICU_DIGITAL_PWD = REG_AHB0_ICU_DIGITAL_PWD & (~0X02);

	REG_AHB0_ICU_CORECLKCON = 0X00;

	REG_AHB0_ICU_CLKSRCSEL = 0X000001b1; //usr 16M,

	REG_AHB0_ICU_ANA_CTL |= (0X01 << 6);

	REG_AHB0_ICU_TIME_COUNT = 0x80000000;

}

uint8_t icu_get_sleep_mode(void)
{
	return default_sleep_mode;
}


void icu_set_sleep_mode(uint8_t v)
{
	default_sleep_mode = v;
}


void enter_deep_sleep(void)
{
	REG_AHB0_ICU_INT_ENABLE = 0;
	REG_AHB0_ICU_IRQ_ENABLE = 0;
	XVR_REG09 = 0X70203C08,  //fix bug for bk3432 in deep sleep mode when use clk 32.768k

	REG_AHB0_ICU_DEEP_SLEEP0=0xffffffff;
	REG_AHB0_ICU_RC32K_CONFG = 0x5a;
	REG_AHB0_ICU_DEEP_SLEEP1&= 0x0000FFFF;
	REG_AHB0_ICU_DEEP_SLEEP1|=( 0x3432<<16);
	Delay_us(10);
	while(1);
}


#if SYSTEM_SLEEP
void cpu_reduce_voltage_sleep(void)
{
	reduce_voltage_set=1;

	REG_AHB0_ICU_DIGITAL_PWD |= 0x80;

	REG_AHB0_ICU_CLKSRCSEL = 0X000001b0;

	REG_AHB0_ICU_DIGITAL_PWD |= SET_CLK48M_PWD;
	REG_AHB0_ICU_DIGITAL_PWD |= SET_CB_BAIS_PWD;
	REG_AHB0_ICU_DIGITAL_PWD |= SET_CLK16M_PWD;//can not power dn 16M, or flash cant nor be read
	REG_AHB0_ICU_DIGITAL_PWD |=SET_HP_LDO_PWD;
	REG_AHB0_ICU_CORECLKCON  = 0x1; // Power down MCU
}


void cpu_wakeup(void)
{
	if(reduce_voltage_set==1)
	{
		reduce_voltage_set=0;
		//REG_AHB0_ICU_INT_ENABLE |= (0x01 << 9);

		REG_AHB0_ICU_DIGITAL_PWD = 0x80;
		REG_AHB0_ICU_CLKSRCSEL  = 0x1b1;
		REG_AHB0_ICU_CPU_STATUS  = 0x551;
	}
}



void cpu_idle_sleep(void)
{
	REG_AHB0_ICU_CORECLKCON = 0x01;//MCU sleep
}


#endif

uint8_t system_reset_reson(void)
{
	if(REG_AHB0_ICU_CPU_STATUS&0x02)
		return 1;//deepsleep wake
	else
		return 0;//power on
}

void switch_clk(uint8_t clk)
{
	REG_AHB0_ICU_DIGITAL_PWD = REG_AHB0_ICU_DIGITAL_PWD & (~0X02);

	if (clk == MCU_CLK_16M )
	{
		REG_AHB0_ICU_CORECLKCON = 0X00; //clk div 0

		REG_AHB0_ICU_CLKSRCSEL = 0X000001b1; //usr 16m,VN=7,VD=3

	}
	else if (clk == MCU_CLK_64M )
	{
		REG_AHB0_ICU_CORECLKCON = 0X00; //clk div 0

		REG_AHB0_ICU_CLKSRCSEL = 0X000005b3; //usr PLL CLK SELT 64M

	}

	else if (clk == MCU_CLK_32M )
	{
		REG_AHB0_ICU_CORECLKCON = 0x04; //clk div 2

		REG_AHB0_ICU_CLKSRCSEL = 0X000005b3; //usr PLL CLK SELT 64M

	}
}





