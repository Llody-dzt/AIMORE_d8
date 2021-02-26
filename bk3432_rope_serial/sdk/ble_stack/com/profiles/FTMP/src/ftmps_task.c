/**
 ****************************************************************************************
 *
 * @file   ftmps_task.c
 *
 * @brief FMTP Server Role Task Implementation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FTMP_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "co_utils.h"
#include "ftmps.h"
#include "ftmps_task.h"
#include "uart.h"
#include "prf_utils.h"
#include "app_ftmp.h"



static uint8_t g_unlockCodeStatus=0x00;

//�˶�״̬
static uint8_t g_trainingStatus = 0x00;

static uint8_t g_controlPointStatus = 0x00;
static uint8_t FitnessMachineStatus = stop_by_user;

//�����룬������ַ����λ
uint8_t g_unlockCode[6] = {0};

//�����豸֧�ֵ�����
static uint8_t g_FitnessMachineFeature[8] = {0};
//�ϱ��豸״̬����
static uint8_t g_Training_Status[2] = {0};
//�ϱ�֧�ֵ��ٶȷ�Χ
static uint8_t g_Supported_Speed_Range[6] = {0};
//�ϱ�֧�ֵ��¶ȷ�Χ
static uint8_t g_Supported_Inclination_Range[6] = {0};
//�ϱ�֧�ֵ�������Χ
static uint8_t g_Supported_Resistance_Level_Range[6] = {0};
//�ϱ�֧�ֵĹ��ʷ�Χ
static uint8_t g_Supported_Power_Range[6] = {0};
//�ϱ�֧�ֵ����ʷ�Χ
static uint8_t g_Supported_Heart_Rate_Range[3] = {0};



void FitnessMachineFeatureRead(uint32_t featureField,uint32_t targetSettingFeatures)
{
#if 0
	uint8_t feature[8]={0x00};
	feature[0]=0x01;
	for(int i=0;i<4;i++)
	{
		feature[i]=(featureField>>(i*8))&0xFF;
		feature[i+4]=(targetSettingFeatures>>(i*8))&0xFF;
	}
	SendUartDataToApp(feature,0x08,FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL);
#else
	for(int i=0;i<4;i++)
	{
		g_FitnessMachineFeature[i]=(featureField>>(i*8))&0xFF;
		g_FitnessMachineFeature[i+4]=(targetSettingFeatures>>(i*8))&0xFF;
	}
#endif
}

//�ϱ�ʵʱ�˶�����

// typedef enum
// {
// 	instantaneous_speed=0x0000, 
// 	treadmill_total_distance =0x0010,
// 	inclination_and_ramp_angle_setting =0x0020,
// 	treadmill_expended_energy =0x0100,
// 	treadmill_elapsed_time =0x0800,
// } treadmill_data_field;

#if 1
void TreadmillDataNotify(treadmill_Data_t data)
{
	uint8_t treadmillData[128]={0x00};
	uint16_t flag=data.flags;

	treadmillData[0]=flag&0xFF;
	treadmillData[1]=(flag>>8)&0xFF;
	int dataIndex=2;
	uint16_t flagBit=0x0001;

	if(1)//(flag&0x0001)==1)
	{
		uint16_t speedData = data.instantaneousSpeed;
		addTwoOctData(treadmillData,speedData,dataIndex);
		dataIndex+=2;				
	}

	for(int i=0;i<15;i++)
	{
		flagBit=0x0001<<i;
	
		if((flag&flagBit)==flagBit)// ����flagÿһλ
		{ 
			switch(i)
			{
				case 1: // Average speed 
				{
					uint16_t averagespeed=data.averageSpeed;
					addTwoOctData(treadmillData,averagespeed,dataIndex);
					dataIndex+=2;
					break;
				}

				case 2: // Instantaneous Cadence 
				{
					uint16_t instantaneousCadence=data.instantaneousCadence;
					addTwoOctData(treadmillData,instantaneousCadence,dataIndex);
					dataIndex+=2;
					break;
				}
				
				case 3: //Average Cadence
				{
					uint16_t averageCadence=data.averageCadence;
					addTwoOctData(treadmillData,averageCadence,dataIndex);
					dataIndex+=2;
					break;
				}
				
				case 4: // Total Distance
				{
					uint32_t distanceData=data.totalDistance;	
					treadmillData[dataIndex]=distanceData&0xFF;
					treadmillData[dataIndex+1]=(distanceData>>8)&0xFF;
					treadmillData[dataIndex+2]=(distanceData>>16)&0xFF;
					dataIndex+=3;
					break;
				}
				
				case 5: // Resistance 
				{
					uint16_t resistance=data.resistanceLevel;
					addTwoOctData(treadmillData,resistance,dataIndex);
					dataIndex+=2;
					break;
				}
				
				case 6: // Instantaneous Power 
				{
					uint16_t instantaneousPower=data.instantaneousPower;
					addTwoOctData(treadmillData,instantaneousPower,dataIndex);
					dataIndex+=2;
					break;
				}
				
				case 7: // Average Power 
				{
					uint16_t averagePower =data.averagePower;
					addTwoOctData(treadmillData,averagePower,dataIndex);
					dataIndex+=2;
					break;
				}
				
				case 8: // Expended Energy
				{
					uint16_t totalEnergy=data.totalEnergy;
					uint16_t energyPerHour=data.energyPerHour;
					uint8_t energyPerMinute=data.energyPerMinute;
					
					addTwoOctData(treadmillData,totalEnergy,dataIndex);
					dataIndex+=2;
					addTwoOctData(treadmillData,energyPerHour,dataIndex);
					dataIndex+=2;
					treadmillData[dataIndex]=energyPerMinute;
					dataIndex+=1;
					break;
				}
				
				case 9: // Heart Rate
				{
					uint8_t heartRate=data.heartRate;
					heartRate=0;
					treadmillData[dataIndex]=heartRate;
					dataIndex+=1;
					break;
				}
				
				case 10: // Metabolic Equivalent
				{
					uint8_t metabolicEquivalent=data.metabolicEquivalent;
					metabolicEquivalent=0;
					treadmillData[dataIndex]=metabolicEquivalent;
					dataIndex+=1;
					break;
				}
				
				case 11: //Elapsed Time
				{
					uint16_t elapsedTime=data.elapsedTime;
					addTwoOctData(treadmillData,elapsedTime,dataIndex);
					dataIndex+=2;
					break;
				}
				
//				case 12: //Remaining Time
//				{
//					uint16_t remainingTime=data.remainingTime;	
//					addTwoOctData(treadmillData,remainingTime,dataIndex);
//					dataIndex+=2;
//					break;
//				}				
				default:
				break;
			}
		}
	}

	// treadmillData[0]=0;
	// treadmillData[1]=0;
    SendUartDataToApp(treadmillData,dataIndex,FTMP_IDX_INDOOR_BIKE_DATA_VAL);
}
#else

void TreadmillDataNotify(treadmill_Data_t data)
{
	uint8_t treadmillData[64]={0x00};
	uint16_t flag=data.flags;
	treadmillData[0]=flag&0xFF;
	treadmillData[1]=(flag>>8)&0xFF;
	int dataIndex=2;

	if((flag&0x0001)==0)
	{
	    uint16_t speedData = (data.instantaneousSpeed); // Instantaneous Speed
	    addTwoOctData(treadmillData,speedData,dataIndex);
	    dataIndex+=2;
	}
	
	uint16_t flagBit=0x0001;

    for(int i=0;i<11;i++)
	{
    	flagBit=0x0001<<i;
		if((flag&flagBit)==flagBit)// ����flagÿһλ
		{ 
    	switch(i)
			{
    		case 1: // Average Speed
    		{
    			uint16_t averageData=data.averageSpeed;
    			addTwoOctData(treadmillData,averageData,dataIndex);
    			dataIndex+=2;
    			break;
    		}
    		case 2: // Total Distance
    		{
    			uint32_t distanceData=data.totalDistance;
    			treadmillData[dataIndex]=distanceData&0xFF;
    			treadmillData[dataIndex+1]=(distanceData>>8)&0xFF;
    			treadmillData[dataIndex+2]=(distanceData>>16)&0xFF;
    			dataIndex+=3;
    			break;
    		}
    		case 3: // Inclination
    		{
    		   uint16_t inclination=data.inclination;
    			 addTwoOctData(treadmillData,inclination,dataIndex);
    			 dataIndex+=2;
				   uint16_t rampAngle=data.rampAngle;
    			 addTwoOctData(treadmillData,rampAngle,dataIndex);
    			 dataIndex+=2;
    			break;
    		}
    		case 4: // Elevation Gain
    		{
    			uint16_t positiveGain=data.positiveElevationGain;
    			uint16_t negativeGain=data.negativeElevationGain;
    			addTwoOctData(treadmillData,positiveGain,dataIndex);
    			dataIndex+=2;
    			addTwoOctData(treadmillData,negativeGain,dataIndex);
    			dataIndex+=2;
    			break;
    		}
    		case 7: // Expended Energy
    		{
    			uint16_t energy=data.totalEnergy;
    			uint16_t energyPerHour=data.energyPerHour;
    			uint8_t energyPerMinute=data.energyPerMinute;
    			addTwoOctData(treadmillData,energy,dataIndex);
    			dataIndex+=2;
    			addTwoOctData(treadmillData,energyPerHour,dataIndex);
    			dataIndex+=2;
    			treadmillData[dataIndex]=energyPerMinute;
    			dataIndex+=1;
    			break;
    		}
    		case 8: // Heart Rate
    		{
    			uint8_t heartRate=data.heartRate;
    			treadmillData[dataIndex]=heartRate;
    			dataIndex+=1;
    			break;
    		}
    		case 10: // Elapsed Time
    		{
    			uint16_t time=data.elapsedTime;
    			addTwoOctData(treadmillData,time,dataIndex);
    			dataIndex+=2;
    			break;
    		}
    		default:
    			break;
    		}
    	}
    }
    SendUartDataToApp(treadmillData,dataIndex,FTMP_IDX_INDOOR_BIKE_DATA_VAL);
}
#endif
void addTwoOctData(uint8_t* treadmillData,uint16_t data,int dataIndex)	
{
	 treadmillData[dataIndex]=data&0xFF;
	 treadmillData[dataIndex+1]=(data>>8)&0xFF;
}

/**
 * CharacteristicsΪfitness_machine_Attributes�����3��Ԫ��
 * ����һ��2���ֽڵ�����
 * ֧��notify
 *
 */
void TrainingStatusRead(uint8_t trainingStatus)
{
	uint8_t status[2]={0x00,0x00};
	status[0]=0x01; // ����չ�ֶ�
	status[1]=trainingStatus; // �ڶ����ֽ�Ϊ״ֵ̬

	g_trainingStatus = trainingStatus;
	
	g_Training_Status[0] = 0x01;
	g_Training_Status[1] = trainingStatus;
	SendUartDataToApp(status,0x02,FTMP_IDX_TRAINING_STATUS_VAL);
}

void SpeedRangeRead(uint16_t minSpeed,uint16_t maxSpeed,uint16_t minIncrement)
{
#if 0
    uint8_t speedRange[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	for(int i=0;i< 2;i++)
	{
		speedRange[i]=(minSpeed>>(i*8))&0xFF;
		speedRange[i+2]=(maxSpeed>>(i*8))&0xFF;
		speedRange[i+4]=(minIncrement>>(i*8))&0xFF;
	}
	return &speedRange[0];
	//SendUartDataToApp(speedRange,0x06,0x04);
#else
	for(int i=0;i< 2;i++)
	{
		g_Supported_Speed_Range[i]=(minSpeed>>(i*8))&0xFF;
		g_Supported_Speed_Range[i+2]=(maxSpeed>>(i*8))&0xFF;
		g_Supported_Speed_Range[i+4]=(minIncrement>>(i*8))&0xFF;
	}
#endif
}

void InclinationRangeRead(uint16_t minInclination,uint16_t maxInclination,uint16_t minIncrement)
{
#if 0
	uint8_t inclinationRange[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	for(int i=0;i< 2;i++){
		inclinationRange[i]=(minInclination>>(i*8))&0xFF;
		inclinationRange[i+2]=(maxInclination>>(i*8))&0xFF;
		inclinationRange[i+4]=(minIncrement>>(i*8))&0xFF;
	}
	//SendUartDataToApp(inclinationRange,0x06,0x05);
#else
	for(int i=0;i< 2;i++)
	{
		g_Supported_Inclination_Range[i]=(minInclination>>(i*8))&0xFF;
		g_Supported_Inclination_Range[i+2]=(maxInclination>>(i*8))&0xFF;
		g_Supported_Inclination_Range[i+4]=(minIncrement>>(i*8))&0xFF;
	}
#endif
}


void ResistanceLevelRangeRead(uint16_t minLevel,uint16_t maxLevel,uint16_t minIncrement)
{
#if 0
	uint8_t resistanceLevelRange[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	for(int i=0;i< 2;i++)
	{
		resistanceLevelRange[i]=(minLevel>>(i*8))&0xFF;
		resistanceLevelRange[i+2]=(maxLevel>>(i*8))&0xFF;
		resistanceLevelRange[i+4]=(minIncrement>>(i*8))&0xFF;
	}
	//SendUartDataToApp(resistanceLevelRange,0x06,0x06);
#else
	for(int i=0;i< 2;i++)
	{
		g_Supported_Resistance_Level_Range[i]=(minLevel>>(i*8))&0xFF;
		g_Supported_Resistance_Level_Range[i+2]=(maxLevel>>(i*8))&0xFF;
		g_Supported_Resistance_Level_Range[i+4]=(minIncrement>>(i*8))&0xFF;
	}
#endif
}


void HeartRateRangeRead(uint8_t minHeartRate,uint8_t maxHeartRate,uint8_t minIncrement)
{
#if 0
	uint8_t heartRateRange[3]={0x00,0x00,0x00};
	heartRateRange[0]=minHeartRate;
	heartRateRange[1]=maxHeartRate;
	heartRateRange[2]=minIncrement;
	SendUartDataToApp(heartRateRange,0x03,0x07);
#else
	g_Supported_Heart_Rate_Range[0]=minHeartRate;
	g_Supported_Heart_Rate_Range[1]=maxHeartRate;
	g_Supported_Heart_Rate_Range[2]=minIncrement;
#endif
}


void FitnessMachineControlPointWrite(uint8_t *pBuf ,uint8_t length)
{
	#ifdef LLODY
		g_controlPointStatus=verify_success;
	#endif
	
	if(pBuf== NULL || length<=0)
	{
	    return;
	}
	
	control_point_target_union param;
	uint8_t opCode=pBuf[0];
	
	if(opCode==request_control && g_unlockCodeStatus==verify_success)// �������Ȩ�ޣ�����Ȩ�ޱ��λΪͬ��
	{ 
		SetControlPermissionToSuccess();
	}

	if(g_controlPointStatus==verify_failed)	// û�����뵽Ȩ�ޣ�����Ӧ����ָ��
	{ 
		FitnessMachineControlPointResponse(opCode,control_not_permitted);
      	return;
	}
	
	if(opCode==reset)// ����Ȩ�޿���ָ��
	{ 
		  ResetControlPermission();
	}
	else
	{		
		uint8_t result= HandleControlPointCollision(opCode);
		
		if(result==operation_failed)
		{
		     FitnessMachineControlPointResponse(opCode,operation_failed);
		     return;
		}
	}

	if(opCode==stop&&(length-1)== 1)
	{
		if(pBuf[1]==0x01)
		{
		    SendAppMachineControlPointToUart(stop,&param);
		}
		else
		{
			SendAppMachineControlPointToUart(pause,&param);
		}
		return;
	}

	if((length-1)== 2)// �в���
	{ 	
		if(opCode==set_target_speed)
		{
			uint16_t speedTarget=0xFF00&(pBuf[2]<<8);
			speedTarget=speedTarget+pBuf[1];
			param.targetSpeed=speedTarget;
		}
		else if(opCode==set_target_inclination)
		{
			uint16_t inclinationTarget=0x7F00&(pBuf[2]<<8);
			inclinationTarget=inclinationTarget+pBuf[1];
			if(0xF0&(pBuf[2]<<8)==0xF0)
			{
				  inclinationTarget=0-inclinationTarget;
			}
			param.targetInclination=inclinationTarget;
		}
	}
	SendAppMachineControlPointToUart(opCode,&param);
}

uint8_t HandleControlPointCollision(uint8_t opCode)
{
  	uint8_t currentStatus=GetFitnessMachineStatus();
	
  	switch(g_trainingStatus)
		{
			case idle: //��Ӧ�ܲ���stop
			{
				return success;
			}
			case quick_start: //��Ӧ�ܲ���start
			{		
				return success;
			}
			
			default:
				return operation_failed;
			break;
//			
//      	case idle:
//			{
//	          	if(opCode==start)	
//				{
//	        	  return success;
//	          	}
//	      		break;
//			}	
//      case quick_start: 
//			{
//	    	  	if(currentStatus==pause_by_user && opCode == pause)
//				{
//	    		     return operation_failed; // ��Ӧʧ��
//	    	    }
//	    	  	return success;
//	    	  	//break;
//			}
//			
//      	case pre_workout:
//			{
//    	  		//return operation_failed;
//    	  		break;
//			}
//      	case post_workout:
//			{
//    	  		//return operation_failed;
//    	  		break;
//			}     
  }
 
}

/**
 * ��������Fitness Machine Control Point����ָ���
 * ����Ҫ����һ��response��app�ࡣ
 *
 */
void FitnessMachineControlPointResponse(uint8_t opCode,uint8_t resultCode)
{
	if(opCode==pause)
	{
		 opCode=stop; // ���ǰ���Э����з�װ
	}
	uint8_t controlPointResponse[3]={0x00,0x00,0x00};
	controlPointResponse[0]=0x80;
	controlPointResponse[1]=opCode;
	controlPointResponse[2]=resultCode;
	SendUartDataToApp(controlPointResponse, 0x03, FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL);
}

/**
 * ��opCodeΪtarget_speed_changed ��target_incline_changedʱparameter��ֵ
 * parameter��С�ֽ�����д���
 *
 */
void FitnessMachineStatusnotify(uint8_t opCode,uint8_t* parameter,uint8_t length)
{
	uint8_t status[3]={0x00,0x00,0x00};

	FitnessMachineStatus = opCode;	// 20200422 hyq
		
	if(opCode==control_permission_lost)
	{
		ResetControlPermission(); // ���ÿ���Ȩ��
	}
	
    if((opCode==target_speed_changed || opCode==target_incline_changed) && length > 1)
	{
		status[0]=opCode;
		status[1]=parameter[0];
		status[2]=parameter[1];
    }
	else
	{
		if(opCode==pause_by_user)
		{
			status[0]=stop_by_user;
			status[1]=0x02;
		}
		else if(opCode==stop_by_user)
		{
			status[0]=opCode;
			status[1]=0x01;
		}
		else
		{
		     status[0]=opCode;
	    }
    }
    SendUartDataToApp(status, 0x03, FTMP_IDX_FITNESS_MACHINE_STATUS_VAL);
}

/**
 * ������Ϊ��չ����
 *
 */
void fitnessExtensionDataWrite(uint8_t* pBuf,uint8_t length)
{
    if(pBuf== NULL || length<=2)
	{
        return;
    }
	huawei_fitness_extension_data_t extensionData;
	uint16_t flags=pBuf[0]+(pBuf[1]<<8);
	extensionData.flags=flags;
	int dataIndex=2;
	uint8_t flagBit=0x01;
	for(int i=0;i<5;i++)
	{
		flagBit=0x01<<i;
		if((flags&flagBit)==flagBit)// ����flagÿһλ
		{ 
			switch(i)
			{
				case 0: // unLockCode,6���ֽ�
				{
					if(length>6)
					{
						addUnlockCode(&extensionData,pBuf,length);
						dataIndex+=6;
					}
				  break;
			  	}
				case 1: // ��������
				{
					if(length>=(dataIndex+1))
					{
						 extensionData.heartRate=pBuf[dataIndex];
						 dataIndex+=1;
					}
					break;
				}
				case 2: // ������
				{
					if(length>=(dataIndex+2))
					{
						uint16_t data=0x0000;
						data=data | pBuf[dataIndex+1];
						data=data<<8;
						data=data | pBuf[dataIndex];
						extensionData.totalEnergy=data;
						dataIndex+=2;
					}
					break;
				}
				case 3: // ��̬����
				{
					if(length>=(dataIndex+2))
					{
						uint16_t data=0x0000;
						data=data | pBuf[dataIndex+1];
						data=data<<8;
						data=data | pBuf[dataIndex];
						extensionData.dynamicEnerty=data;
						dataIndex+=2;
					}
					break;
				}
				default: // �ܲ���
				{
					if(length>=(dataIndex+2))
					{
						uint16_t data=0x0000;
						data=data | pBuf[dataIndex+1];
						data=data<<8;
						data=data | pBuf[dataIndex];
						extensionData.steps=data;
						dataIndex+=2;
					}
					break;
			 	}
			}
		}
	}
	SendAppExtensionDataToUart(&extensionData);
}
/**
 * ����������
 *
 */
void addUnlockCode(huawei_fitness_extension_data_t *extensionData,uint8_t* pBuf,uint8_t length)
{
	if(length<=0x06)
	{
		return;
	}
	uint8_t unlockCode[6]={0x00};
	for(int i=0;i<6;i++)
	{
		unlockCode[i]=pBuf[i+2];
	}
	VerifyUnlockCode(unlockCode); // У�������
	memcpy((*extensionData).unLockCode,unlockCode,sizeof(unlockCode));
}

/**
 * �����Ͽ�����øú���
 * ���ÿ���������Ȩ״̬
 */
void ResetControlPermission()
{
	g_controlPointStatus=verify_failed;
}

/**
 * �����Ͽ�����øú���
 *
 */

void SetControlPermissionToSuccess()
{
	g_controlPointStatus=verify_success;
	
}

void VerifyUnlockCode(uint8_t *unlockCode)
{
    if(CheckUnlockCode(unlockCode) == verify_success)
	{
    	 g_unlockCodeStatus=verify_success; // ������һ�£�������Ϊ0xFF
    }
}

/**
 * �����Ͽ�����øú���
 * ���ý�����У��״̬
 *
 */
void ResetUnlockPermission()
{
	 g_unlockCodeStatus=verify_failed;
}
void ResetWhenBluetoothDisconnect()
{
	ResetControlPermission();
	ResetUnlockPermission();
}


/**
 * ��ȡ�ܲ�����ǰ״̬,���������ɸ���
 * ״̬��fitness_machine_status_operation_code����
 * ���û�ͨ������������ܲ���״̬��MCU��״̬ͬ��������ģ�顣
 *
 */
uint8_t GetFitnessMachineStatus()
{
   //return stop_by_user;
   return FitnessMachineStatus;	// 20200422 hyq
}

/**
 * У�������ĺ���,���������ɸ���
 * �����ڴ˺����жϽ���������õĽ������Ƿ�һ��
 * ������У��������ʵ����������
 *
 */
uint8_t CheckUnlockCode(uint8_t *unlockCode)
{
	uint8_t i;
#if 1
	UART_PRINTF("rx:");
	for(i=0; i<6; i++)
	{
		UART_PRINTF("%x", unlockCode[i]);
	}

	UART_PRINTF("\r\nmy:");
	for(i=0; i<6; i++)
	{
		UART_PRINTF("%x", g_unlockCode[i]);
	}
#endif
	if(memcmp(unlockCode, g_unlockCode, 6) == 0)
	{
		UART_PRINTF("\r\nUnlockCode success\r\n");
   		return verify_success;
	}
	else
	{
		UART_PRINTF("\r\nUnlockCode failed\r\n");
   		return verify_failed;
	}
}


//static int ftmps_fff1_level_upd_req_handler(ke_msg_id_t const msgid,
//                                            struct ftmps_fff1_level_upd_req const *param,
//                                            ke_task_id_t const dest_id,
//                                            ke_task_id_t const src_id)
//{
//    int msg_status = KE_MSG_SAVED;
//    uint8_t state = ke_state_get(dest_id);
//	
//    // check state of the task
//    if(state == FTMPS_IDLE)
//    {
//        struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);

//        // put task in a busy state
//        ke_state_set(dest_id, FTMPS_BUSY);						
//		    ftmps_notify_fff1_lvl(ftmps_env, param);
//		    ke_state_set(dest_id, FTMPS_IDLE);   
//		    msg_status = KE_MSG_CONSUMED;	
//    }

//    return (msg_status);
//  }

static int ftmps_level_upd_req_handler(ke_msg_id_t const msgid,
                                            struct ftmps_level_upd_req const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_SAVED;
    uint8_t state = ke_state_get(dest_id);
	
    // check state of the task
    if(state == FTMPS_IDLE)
    {
        struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);

        // put task in a busy state
        ke_state_set(dest_id, FTMPS_BUSY);						
		ftmps_notify_lvl(ftmps_env, param);
		ke_state_set(dest_id, FTMPS_IDLE);   
		msg_status = KE_MSG_CONSUMED;	
    }

    return (msg_status);
  }
  
static int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint16_t  att_idx = 0;
    // retrieve handle information
    uint8_t status = ftmps_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(//(att_idx == FTMP_IDX_TREADMILL_DATA_NTY_CFG)
				     //||(att_idx == FTMP_IDX_CROSS_TRAINER_DATA_NTY_CFG)
				     //||(att_idx == FTMP_IDX_STEP_CLIMBER_DATA_NTY_CFG)
						 (att_idx == FTMP_IDX_TRAINING_STATUS_NTY_CFG)
						 ||(att_idx == FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG)
						 ||(att_idx == FTMP_IDX_FITNESS_MACHINE_STATUS_NTF_CFG)
						)
        {
            // CCC attribute length = 2
            cfm->length = 2;
        }
        // not expected request
        else
        {
            cfm->length = 0;
            status = ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}



static int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint16_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = ftmps_get_att_idx(param->handle,  &att_idx);
	
	//UART_PRINTF("gattc_write: %d, %d\r\n", att_idx, param->length);
		
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);
       
        // Extract value before check
        uint16_t ntf_cfg = co_read16p(&param->value[0]);
        // Only update configuration if value for stop or notification enable
        if (0)//(att_idx == FTMP_IDX_TREADMILL_DATA_NTY_CFG))     
        {
         	if(param->length == 2)
			{
				ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][0] = param->value[0];  
				ftmps_env->FTMP_TREADMILL_DATA_NTY_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			}
		}
#if 0
		else if(att_idx == FTMP_IDX_CROSS_TRAINER_DATA_NTY_CFG)
		{	 
          	if(param->length == 2)
			{
			   	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][0] = param->value[0];
		       	ftmps_env->FTMP_CROSS_TRAINER_DATA_NTY_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			} 
		}
		else if(att_idx == FTMP_IDX_STEP_CLIMBER_DATA_NTY_CFG)
		{
           	if(param->length == 2)
			{
			    ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][0] = param->value[0];
		     	ftmps_env->FTMP_STEP_CLIMBER_DATA_NTY_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			}
		}	 
#endif
		else if(att_idx == FTMP_IDX_TRAINING_STATUS_NTY_CFG)
		{					  
           	if(param->length == 2)
			{
			   	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][0] = param->value[0];
		       	ftmps_env->FTMP_TRAINING_STATUS_NTY_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			}   
		}
		else if(att_idx == FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG)
		{	 
          	if(param->length == 2)
			{
			   	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][0] = param->value[0];
		       	ftmps_env->FTMP_FITNESS_MACHINE_CONTROL_POINTE_INDIC_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			} 
		}
		else if(att_idx == FTMP_IDX_FITNESS_MACHINE_STATUS_NTF_CFG)
		{	 
           	if(param->length == 2)
			{
			    ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][0] = param->value[0];
		       	ftmps_env->FTMP_FITNESS_MACHINE_STATUS_NTF_CFG[conidx][1] = param->value[1];
			}
			else
			{
			  	status = PRF_APP_ERROR;
			}
	 	}	
		else if ((att_idx == FTMP_IDX_FITNESS_MACHINE_CONTROL_POINTE_VAL)||(att_idx == FTMP_IDX_FITNESS_EXTENSION_DATA_VAL))
		{
			// Allocate the alert value change indication
			struct  ftmps_writer_ind *ind = KE_MSG_ALLOC(FTMPS_WRITER_REQ_IND,
			        prf_dst_task_get(&(ftmps_env->prf_env), conidx),
			        dest_id, ftmps_writer_ind);
			
			// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			ind->flag = att_idx;
				
			// Send the message
			ke_msg_send(ind);
		}
	    else
		{
			status = PRF_APP_ERROR;
		}

    }

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}   



static int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm * cfm;
    uint16_t  att_idx = 0;
    //uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = ftmps_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
	uint8_t *value = NULL;
    struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);

	//UART_PRINTF("gattc_read: %d\r\n", att_idx);
	
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
    	if(att_idx == FTMP_IDX_FITNESS_MACHINE_FEATURE_VAL)
		{
		   	length = 8;
			value = g_FitnessMachineFeature;
		}
		else if(att_idx == FTMP_IDX_TRAINING_STATUS_VAL)
		{
		   	length = 2;
			value = g_Training_Status;
		}
		else if(att_idx == FTMP_IDX_SUPPORT_SPEED_RANGE_VAL)
		{
		   	length = 6;
			value = g_Supported_Speed_Range;
		}
		else if(att_idx == FTMP_IDX_SUPPORT_INCLINATION_RANGE_VAL)
		{
     		length = 6;
			value = g_Supported_Inclination_Range;
		}
		else if(att_idx == FTMP_IDX_SUPPORT_RESISTANCE_LEVEL_RANGE_VAL)
		{
     		length = 6;
			value = g_Supported_Resistance_Level_Range;
		}
		else if(att_idx == FTMP_IDX_SUPPORT_POWER_RANGE_VAL)
		{
     		length = 6;
			value = g_Supported_Power_Range;
		}
		else if(att_idx == FTMP_IDX_SUPPORT_HEART_RATE_RANGE_VAL)
		{
     		length = 3;
			value = g_Supported_Heart_Rate_Range;
		}
		else
		{
		 	length = 0;
			value = NULL;
		} 
    }

    //Send write response
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    cfm->handle = param->handle;
    cfm->status = status;
    cfm->length = length;
   
    if (status == GAP_ERR_NO_ERROR)
    {
		if(value != NULL)
		{
			memcpy(cfm->value,value,length);
		}
        else
        {
            /* Not Possible */
        }
    }
	
    ke_msg_send(cfm);
		
    return (KE_MSG_CONSUMED);
}   


static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
#if 0
    if(param->operation == GATTC_NOTIFY)
    {	
      	uint8_t conidx = KE_IDX_GET(src_id);
      	struct ftmps_env_tag* ftmps_env = PRF_ENV_GET(FTMPS, ftmps);

	    struct ftmps_level_upd_rsp *rsp = KE_MSG_ALLOC(FTMPS_LEVEL_UPD_RSP,
                                          prf_dst_task_get(&(ftmps_env->prf_env), conidx),
                                          dest_id, ftmps_level_upd_rsp);

        rsp->status = param->status;			
        ke_msg_send(rsp);
    }
#endif
	// go back in to idle mode
    ke_state_set(dest_id, ke_state_get(dest_id) & ~FTMPS_BUSY);
	
    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler ftmps_default_state[] =
{
    //{FTMPS_FFF1_LEVEL_UPD_REQ,      (ke_msg_func_t) ftmps_fff1_level_upd_req_handler},
	{FTMPS_LEVEL_UPD_REQ,           (ke_msg_func_t) ftmps_level_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler ftmps_default_handler = KE_STATE_HANDLER(ftmps_default_state);

#endif /* #if (BLE_FMTP_SERVER) */


