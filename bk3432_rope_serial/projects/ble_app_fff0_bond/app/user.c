/**
 ****************************************************************************************
 *
 * @file app_fff0.c
 *
 * @brief fff0 Application Module entry point
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

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
//#include "app_fff0.h"              // Battery Application Module Definitions
#include "app.h"                    // Application Definitions
#include "app_task.h"             // application task definitions
//#include "fff0s_task.h"           // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"             // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
//#include "fff0s.h"
#include "ke_timer.h"
#include "uart.h"
#include "flash.h"
#include "prf_utils.h"
#include "user_queue.h"
#include "stdio.h"
#include "stdlib.h"
#include "gpio.h"
//#include "wdt.h"
#include "user.h"
#include "flash.h"
#include "co_math.h"
#include "ftmps_task.h"
#include "uart2.h"

#define USER_QUEUE_MAX_LEN   512
USER_QUEUE_T tcuartrxqueue;
USER_QUEUE_T tcbletxqueue;


uint8_t tcuartrxqueuebuf[USER_QUEUE_MAX_LEN];
uint8_t tcbletxqueuebuf[USER_QUEUE_MAX_LEN];

uint8_t tcuartrxcompelet = 2;	//0:未接收完，1:接收完，2:无数据
uint8_t tcbletxstate = 0;	

uint16_t tcblemaxmtu = 20;

uint8_t tcbleconnstate = 0;

uint8_t tcbletxdata[128];
uint16_t tcbletxlen = 0;


uint8_t g_check_rand = 0;
uint8_t g_inquire_cmd = 0;
uint8_t g_mcu_status = 0;	//发送给mcu的状态
uint8_t g_mcu_speed = 0;	//发送给mcu的速度


TC_USER_CONFIG_S tcuserconfig;

void user_init(void)
{
	User_Queue_Init(&tcuartrxqueue,tcuartrxqueuebuf,USER_QUEUE_MAX_LEN);
	User_Queue_Init(&tcbletxqueue,tcbletxqueuebuf,USER_QUEUE_MAX_LEN);
	
	apptcuserconfigread();
}

//清空变量，防止上电时串口收到数据会打乱流程
void app_clear(void)
{
	tcuartrxcompelet = 2;
	User_Queue_Clear(&tcuartrxqueue);
	User_Queue_Clear(&tcbletxqueue);
}

void apptcbletxhandle(uint8_t status)
{
	if(!tcbleconnstate)
	{
		User_Queue_Data_Del(&tcbletxqueue,User_Queue_Get_Len(&tcbletxqueue));
		tcbletxlen = 0;
		return;
	}
	
	if(!status)
	{
		tcbletxlen = User_Queue_Get_Len(&tcbletxqueue);
		if(tcbletxlen >= tcblemaxmtu)
			tcbletxlen = tcblemaxmtu;

		if(tcbletxlen)
			User_Queue_Data_Out(&tcbletxqueue,tcbletxdata,tcbletxlen);
	}

	if(tcbletxlen)
	{
		//app_fff0_notify(0,FFF0_IDX_CHA_FFF1_VAL,tcbletxdata,tcbletxlen);
		//appm_write_uuid_data_req(0xfff1, tcbletxlen, tcbletxdata);
		tcbletxstate = 1;
	}
	else
		tcbletxstate = 0;
}
void apptcblerxhandle(uint8_t *data, uint16_t len)
{
	apptcuarttxdata(data,len);
}
void apptcuserconfigread(void)
{
	flash_read(0,0x27800>>2,sizeof(TC_USER_CONFIG_S),(uint8_t *)&tcuserconfig);
	
	if(tcuserconfig.configflag != 0xF0)
	{		
		apptcuserconfiginit();
		
		flash_erase(0,0x27800>>2,0x400);
		flash_write(0,0x27800>>2,sizeof(TC_USER_CONFIG_S),(uint8_t *)&tcuserconfig);
	}

#if 0	
	UART_PRINTF("pair: %d\r\n", tcuserconfig.pair);
	if(tcuserconfig.pair)
	{
		uart_printf("save addr = %02x:%02x:%02x:%02x:%02x:%02x\r\n",tcuserconfig.addr[0],tcuserconfig.addr[1],tcuserconfig.addr[2],
	 		            tcuserconfig.addr[3],tcuserconfig.addr[4],tcuserconfig.addr[5]);
	}
#endif
}

void apptcuserconfig_write(void)
{
	flash_erase(0,0x27800>>2,0x400);
	flash_write(0,0x27800>>2,sizeof(TC_USER_CONFIG_S),(uint8_t *)&tcuserconfig);
}

void apptcuserconfiginit(void)
{
	uint8_t temp[6];
	flash_read(0,0x27ff0>>2,6,temp);
	
	tcuserconfig.configflag = 0xF0;
	
	memset(tcuserconfig.ver,0x00,sizeof(tcuserconfig.ver));
	memcpy(tcuserconfig.ver,"XZX_TC_5.0.0_20200331",strlen("XZX_TC_5.0.0_20200331"));
	
	//memset(tcuserconfig.name,0x00,sizeof(tcuserconfig.name));
	//memcpy(tcuserconfig.name,"F9398_SPP",strlen("F9398_SPP"));
	
	tcuserconfig.uart = 2;	//9600
}

void user_send_connect_cmd(void)
{
	//uart_send("AT+GCON:1\r\n", 11);
}

void apptcuartrxcompeletset(uint8_t status)
{
	tcuartrxcompelet = status;
}
void apptcuartrxdatain(uint8_t *data, uint16_t len)
{
	User_Queue_Data_In(&tcuartrxqueue,data,len);
}
void apptcuarttxdata(uint8_t *data, uint16_t len)
{
	uart_send(data,len);	
}
void apptcconnhandle(void)
{
	User_Queue_Data_Del(&tcbletxqueue,User_Queue_Get_Len(&tcbletxqueue));
	tcbletxstate = 0;
	tcbleconnstate = 1;

	//uart_send("AT+SPAI:1\r\n", 11);
}
void apptcdisconnhandle(void)
{
	User_Queue_Data_Del(&tcbletxqueue,User_Queue_Get_Len(&tcbletxqueue));;
	tcbletxstate = 0;
	tcbleconnstate = 0;

	//uart_send("AT+GCON:0\r\n", 11);
}


//处理串口数据
void apptc_uart_rx_handle(void)
{
	if(1 != tcuartrxcompelet)		//没接收完不处理
		return;		
	
	/*******************************************************
	//处理串口来的数据
	********************************************************/
	uint8_t uart_rx_buf[USER_QUEUE_MAX_LEN];
	uint16_t uart_rx_len;

	uart_rx_len = User_Queue_Get_Len(&tcuartrxqueue);
	if(uart_rx_len)
	{
		if(tcuartrxcompelet == 1)	//接收完毕
		{
	    	User_Queue_Data_Out(&tcuartrxqueue,uart_rx_buf,uart_rx_len);			
			apptc_data_decode(uart_rx_buf, uart_rx_len);
			tcuartrxcompelet = 2;
		}
	}

	//uart_printf("AA:%d\r\n", uart_rx_len);
}

//A9 08 04 0E 05 13 46 FB
void apptc_data_decode(uint8_t *data, uint8_t len)
{
	uint8_t i, index=0, size, sum=0;
	if(data[0] != 0xA9)
		return;

#if 0
	if(data[1] != 0x02)
	{
		UART_PRINTF("rx data: ");
		for(i=0; i<len; i++)
		{
			UART_PRINTF("%02x ", data[i]);
		}
		UART_PRINTF("\r\n");
	}
#endif

	while(1)
	{
		size = data[index+2];

		for(i=0; i<size+3; i++)
		{
			sum ^= data[index+i];
		}
		//UART_PRINTF("sum: %x,%x\r\n", sum, data[index+size+3]);
		if(sum == data[index+size+3])
		{
			apptc_cmd_decode(data[index+1], &data[index+3], data[index+2]);
		}

		//UART_PRINTF("AA: %d,%d\r\n", len, size+4 +index);
		if(len > (size+4 +index))
		{
			sum = 0;
			index = size+4;		//一包数据，有两个指令
		}
		else
		{
			return;
		}
	}
}


/*
tx data: a9 1e 01 fe 48 
rx data: a9 1e 05 07 01 10 01 30 95 

tx data: a9 f2 01 2f 75 
rx data: a9 f2 03 01 00 00 59 

tx data: a9 0a 01 06 a4 
rx data: a9 0a 04 0a 8c 00 00 21
*/

//MCU上报的数据是大端字节序
//data[0] = M>>8 & 0xff;
//data[1] = M & 0xff;
void apptc_cmd_decode(uint8_t cmd, uint8_t *data, uint8_t len)
{
	//UART_PRINTF("cmd: 0x%02x, %d\r\n", cmd, len);

	switch(cmd)
	{
		case 0x02:
			{
				uint8_t status = data[12];
				treadmill_Data_t send;
				
				send.flags = instantaneous_speed | treadmill_total_distance 
					| treadmill_expended_energy | treadmill_elapsed_time;
				
				send.instantaneousSpeed = data[11] *10;			//瞬时速度
				send.totalDistance = (data[4]<<8) + data[5];	//总距离
				send.totalEnergy = ((data[1]<<16) + (data[2]<<8) +data[3]) /100;	//卡路里
				send.elapsedTime = (data[9]<<8) + data[10];		//运行时间

				#if 0
				if(send.instantaneousSpeed > 0)
				{
					UART_PRINTF("AA: %d, %d, %d, %d\r\n", send.instantaneousSpeed, send.totalDistance,
						send.totalEnergy, send.elapsedTime);
				}
				#endif
				TreadmillDataNotify(send);

				status >>= 5;
				if(status != g_mcu_status)
				{
					g_mcu_status = status;
					UART_PRINTF("mcu status:%d\r\n", g_mcu_status);
					if(g_mcu_status == 0)
					{
						TrainingStatusRead(idle);
					}
					else if(g_mcu_status == 2)
					{
						TrainingStatusRead(quick_start);
						FitnessMachineStatusnotify(start_by_user, NULL, 0);
					}
					else if(g_mcu_status == 5)
					{
						
					}
				}
			}
			break;
		case 0x03:	//错误码指令
			{
			}
			break;
		case 0x05:	//电子表按键指令
			{
				if(data[0] == 0x01)	//开始跑步机
				{
					UART_PRINTF("key start\r\n");
					g_mcu_status = 0;
					TrainingStatusRead(pre_workout);
				}
				else if(data[0] == 0x02)	//停止跑步机
				{
					UART_PRINTF("key stop\r\n");
					TrainingStatusRead(post_workout);
					FitnessMachineStatusnotify(stop_by_user, NULL, 0);
				}
			}
			break;
		case 0x09:	//跑步机状态变更
			{
				if(data[0] == 0x01)	//3秒倒计时启动
				{
				}
				else if(data[0] == 0x02)	//运行中 
				{
				}
				else if(data[0] == 0x03)	//即将暂停
				{
				}
				else if(data[0] == 0x04)	//暂停
				{
				}
				else if(data[0] == 0x05)	//即将停止
				{
				}
				else if(data[0] == 0x06)	//停止
				{
				}

				uint8_t id = data[0];
				apptc_cmd_encode(0x8E, &id, 1);
			}
			break;
		case 0x08:
			{
				if(len == 1)
				{
					if(data[0] == 0xff)
						ke_timer_set(APP_CHECK_DRIVE_TIMER, TASK_APP, 10);
				}
				else if(len == 4)
				{
					if(user_check_drive(data, len) == 1)
					{
						g_inquire_cmd = 0x1E;
						ke_timer_set(APP_INQUIRE_TIMER, TASK_APP, 10);
					}
				}
			}
			break;
		case 0x0A:	//速度坡度范围
			{
				//a9 0a 04 0a 8c 00 00 21
				//5 == 0.5km/h
				//华为上层单位是0.01km/h, 1 == 0.01km/h
				
				UART_PRINTF("speed range: %d,%d\r\n", data[0], data[1]);
				if((data[0] != 0xff) && (data[1] != 0xff))
					SpeedRangeRead(data[0]*10, data[1]*10, 10);
				else
					SpeedRangeRead(0x0a*10, 0x8c*10, 10);
			}
			break;
		case 0x1E:	//设备型号信息
			{
				//a9 1e 05 07 01 10 01 30 95 
			}
			break;
		case 0xE0:	//速度变化
			{
				
			}
			break;
		case 0xF2:	//告知公英制、 当前速度和坡度值
			{
				//a9 f2 03 01 00 00 59
				//01 公制
			}
			break;
		default:

			break;
	}
}

uint8_t user_check_drive(uint8_t *data, uint8_t len)
{
	uint8_t index = g_check_rand % 6;
	uint8_t flag = 0;

	switch(index)
	{
		case 0:
		{
			if((data[2] == ((data[0]+data[1])&0xff))
				&& (data[3] == ((data[0]*data[1])&0xff)))
			{
				flag = 1;
			}
		}
		break;
		case 1:
		{
			if((data[2] == ((data[0]*data[1])&0xff))
				&& (data[3] == ((data[0]+data[1])&0xff)))
			{
				flag = 1;
			}
		}
		break;
		case 2:
		{
			if((data[0] == ((data[1]+data[2])&0xff))
				&& (data[3] == ((data[1]*data[2])&0xff)))
			{
				flag = 1;
			}
		}
		break;
		case 3:
		{
			if((data[0] == ((data[1]*data[2])&0xff))
				&& (data[3] == ((data[1]+data[2])&0xff)))
			{
				flag = 1;
			}
		}
		break;
		case 4:
		{
			if((data[0] == ((data[2]+data[3])&0xff))
				&& (data[1] == ((data[2]*data[3])&0xff)))
			{
				flag = 1;
			}
		}
		break;
		case 5:
		{
			if((data[0] == ((data[2]*data[3])&0xff))
				&& (data[1] == ((data[2]+data[3])&0xff)))
			{
				flag = 1;
			}
		}
		break;
	}

	if(flag == 1)
	{
		UART_PRINTF("check_drive ok\r\n");
	}
	else
	{
		UART_PRINTF("check_drive fail\r\n");
		ke_timer_set(APP_CHECK_DRIVE_TIMER, TASK_APP, 100); 
		return 0;
	}

	uint8_t A, B;
	uint8_t send[4];

	A = co_rand_byte();
	B = co_rand_byte();
	index = data[3] % 6;

	switch(index)
	{
		case 0:
		{
			send[0] = A;
			send[1] = B;
			send[2] = (A + B) & 0xff;
			send[3] = (A * B) & 0xff;
		}
		break;
		case 1:
		{
			send[0] = A;
			send[1] = B;
			send[2] = (A * B) & 0xff;
			send[3] = (A + B) & 0xff;
		}
		break;
		case 2:
		{
			send[0] = (A + B) & 0xff;
			send[1] = A;
			send[2] = B;
			send[3] = (A * B) & 0xff;
		}
		break;
		case 3:
		{
			send[0] = (A * B) & 0xff;
			send[1] = A;
			send[2] = B;
			send[3] = (A + B) & 0xff;
		}
		break;
		case 4:
		{
			send[0] = (A + B) & 0xff;
			send[1] = (A * B) & 0xff;
			send[2] = A;
			send[3] = B;
		}
		break;
		case 5:
		{
			send[0] = (A * B) & 0xff;
			send[1] = (A + B) & 0xff;
			send[2] = A;
			send[3] = B;
		}
		break;
	}

	apptc_cmd_encode(0x08, send, 4);

	ke_timer_set(APP_HEARTBEAT_TIMER, TASK_APP, 50);
	return 1;
}


//向mcu发送心跳包
int app_heartbeat_timer_handler( ke_msg_id_t const msgid,
									 void *param,
									 ke_task_id_t const dest_id,
									 ke_task_id_t const src_id )
{
	//UART_PRINTF("heartbeat\r\n");
	uint8_t heartbeat[3];

	heartbeat[0] = g_mcu_status;
	heartbeat[1] = g_mcu_speed;
	heartbeat[2] = 0;
	apptc_cmd_encode(0xA0, heartbeat, 3);	  

	ke_timer_set(APP_HEARTBEAT_TIMER, TASK_APP, 100);
	return KE_MSG_CONSUMED;
}

//握手
int app_check_drive_timer_handler( ke_msg_id_t const msgid,
                                     void *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id )
{
	g_check_rand = co_rand_byte();
	UART_PRINTF("check_drive: %d\r\n", g_check_rand);
	
	apptc_cmd_encode(0x08, &g_check_rand, 1);	  
	
    return KE_MSG_CONSUMED;
}

//查询mcu信息
int app_inquire_timer_handler( ke_msg_id_t const msgid,
                                     void *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id )
{
	UART_PRINTF("inquire_cmd: 0x%02x\r\n", g_inquire_cmd); 
	
	if(g_inquire_cmd == 0x1E)
	{
		uint8_t status = 0xFE;
		apptc_cmd_encode(0x1E, &status, 1);

		g_inquire_cmd = 0xF2;
		ke_timer_set(APP_INQUIRE_TIMER, TASK_APP, 10);
	}
	else if(g_inquire_cmd == 0xF2)
	{
		uint8_t status = 0x2F;
		apptc_cmd_encode(0xF2, &status, 1);
		
		g_inquire_cmd = 0x0A;
		ke_timer_set(APP_INQUIRE_TIMER, TASK_APP, 10);
	}
	else if(g_inquire_cmd == 0x0A)
	{
		uint8_t status = 0x06;
		apptc_cmd_encode(0x0A, &status, 1);
	}

    return KE_MSG_CONSUMED;
}

void apptc_cmd_encode(uint8_t cmd, uint8_t *data, uint8_t len)
{
	uint8_t send[48];
	uint8_t i, sum = 0;

	send[0] = 0xA9;
	send[1] = cmd;
	send[2] = len;
	
	for(i=0; i<len; i++)
	{
		send[3+i] = data[i];
	}
	
	for(i=0; i<len +3; i++)
	{
		sum ^= send[i];
	}

	send[3 +len] = sum;

#if 0
	UART_PRINTF("tx data: ");
	for(i=0; i<len+4; i++)
	{
		UART_PRINTF("%02x ", send[i]);
	}
	UART_PRINTF("\r\n");
#endif
	uart_send(send, len+4);
}




