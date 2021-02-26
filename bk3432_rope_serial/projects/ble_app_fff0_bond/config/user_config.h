/**
 *******************************************************************************
 *
 * @file user_config.h
 *
 * @brief Application configuration definition
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *******************************************************************************
 */
 
#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


 
 /******************************************************************************
  *############################################################################*
  * 							SYSTEM MACRO CTRL                              *
  *############################################################################*
  *****************************************************************************/

//�����Ҫʹ��GPIO���е��ԣ���Ҫ�������
#define GPIO_DBG_MSG					0
//UARTʹ�ܿ��ƺ�
#define UART_PRINTF_EN					1
//����Ӳ�����Կ���
#define DEBUG_HW						0


#define UART_CHANGE		0

#define HILINK		1

#define LLODY	 //2020.10.29���Դ���-APP����Э��
#define D8LLODY  //2020.11.24 D8

/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 *******************************************************************************/
 
//���Ӳ������¿���
#define UPDATE_CONNENCT_PARAM  			1

//��С���Ӽ��
#define BLE_UAPDATA_MIN_INTVALUE		20
//������Ӽ�� 
#define BLE_UAPDATA_MAX_INTVALUE		40
//����Latency
#define BLE_UAPDATA_LATENCY				0
//���ӳ�ʱ
#define BLE_UAPDATA_TIMEOUT				600


//�豸����
#define APP_DFLT_DEVICE_NAME           ("BK3432-GATT")


 //�㲥��UUID����
#ifdef LLODY
	#define APP_FTMP_ADV_SERVICE_UUID        "\x03\x03\x26\x18"
	#define APP_FTMP_ADV_SERVICE_UUID_LEN    (4)

	#define APP_FTMP_ADV_DATA_UUID     			 "\x06\x16\x26\x18\x01\x00\x20"
	#define APP_FTMP_ADV_DATA_UUID_LEN   		 (7)

	#define APP_FTMP_ADV_SERDATA_UUID      		"\x08\xff\x7D\x02\x01\x05\x00\xff\xff"
	#define APP_FTMP_ADV_SERDATA_UUID_LEN    	(9)
#else
	#define APP_FFF0_ADV_DATA_UUID        "\x03\x03\xF0\xFF"
	#define APP_FFF0_ADV_DATA_UUID_LEN    (4)
#endif

//#define APP_ADV_MANU_SPECIFIC_DATA   		"\x08\xff\x7D\x02\x01\x05\x00\xff\xff"
//#define APP_ADV_MANU_SPECIFIC_DATA_LEN    	(9)
//ɨ����Ӧ������
//#define APP_SCNRSP_DATA        "\x0c\x08\x42\x4B\x33\x34\x33\x32\x2D\x41\x4E\x43\x53" //BK3432-ANCS"
//#define APP_SCNRSP_DATA_LEN     (13)


//�㲥��������
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         (80)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         (80)
/// Fast advertising interval
#define APP_ADV_FAST_INT        (32)




/*******************************************************************************
 *#############################################################################*
 *								DRIVER MACRO CTRL                              *
 *#############################################################################*
 ******************************************************************************/

//DRIVER CONFIG
#define UART_DRIVER						1
#define UART2_DRIVER					0
#define GPIO_DRIVER						1
#define AUDIO_DRIVER					0
#define RTC_DRIVER						0
#define ADC_DRIVER						1
#define I2C_DRIVER						0
#define PWM_DRIVER						1





#endif /* _USER_CONFIG_H_ */
