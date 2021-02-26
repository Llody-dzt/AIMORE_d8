#include "app_fff0.h" 
#include "rope.h"
#include "rtc.h"
#include "app_dis.h" 
#include "uart.h" 
extern void app_fff1_send_lvl(uint8_t* buf, uint8_t len);
uint8_t bat_level[1];
typedef enum
{
	DATA_TIME = 1,
	BATT_STATU,
	ROPE_CTRL,
	DEV_INFO,
}ROP_CONTROL;
typedef enum
{
	START_F_ROPE = 1,
	START_T_CNT,
	START_T_TIME,
	PAUSE,
	RECOVERY,
	STOP,
	START_SYNC_HISTORY,
	END_SYNC_DEL_HISTORY,
}ROPE;
typedef enum
{
	ROPE_REAL_DATA = 4,
	ROPE_HISTORY_DATA,
	MANUFACTURER,
	MODEL_NB,
	SERIAL_NB,
	SW_REV,
	HARD_REV,
}ROPE_DATA_CMD;
void Rop_Data_Send(uint8_t Command,uint8_t *Parameter, uint8_t l)
{
	uint8_t  send_buffer[20] ={0};
  send_buffer[0] = Command;
	memcpy(send_buffer+1,Parameter,l);
	app_fff1_send_lvl(send_buffer, l+1);
	UART_PRINTF("%d%s\r\n",send_buffer[0],Parameter);
}
void Rop_Control(char *buf, int l)
{
	uint8_t len;
	uint8_t *data = NULL;
	char Command = (ROP_CONTROL)buf[0];
	switch(Command)
	{
		case DATA_TIME://unix时间格式（1970年1月1日 00:00到现在的秒数）
			
			break;
		case BATT_STATU://读取电量，剩余电量百分比
		{	
			//上报电量
			bat_level[0]= 100;
			data= &bat_level[0];
			len = 1;
			Rop_Data_Send(BATT_STATU,data,len);
		}
			break;
		case ROPE_CTRL:
		{
			char Code = buf[1];
	    switch(Code)
			{
				case START_F_ROPE://启动自由跳绳
		    
				break;
				case START_T_CNT://启动定时计数
		
				break;
				case START_T_TIME://启动定数计时
		
				break;
				case PAUSE://暂停
		
				break;
				case RECOVERY://恢复
		
				break;
				case STOP://停止
		
				break;
				case START_SYNC_HISTORY://启动同步历史记录
		
				break;
				case END_SYNC_DEL_HISTORY://结束同步删除历史记录
		
				break;
			}
		}	
		break;
		case DEV_INFO://读取设备信息
			  
        len = APP_DIS_MANUFACTURER_NAME_LEN;
        data = (uint8_t *)APP_DIS_MANUFACTURER_NAME;
				Rop_Data_Send(MANUFACTURER,data,len);
				
        len = APP_DIS_MODEL_NB_STR_LEN;
        data = (uint8_t *)APP_DIS_MODEL_NB_STR;
				Rop_Data_Send(MODEL_NB,data,len);
				
				len = APP_DIS_SERIAL_NB_STR_LEN;
				data = (uint8_t *)APP_DIS_SERIAL_NB_STR;
				Rop_Data_Send(SERIAL_NB,data,len);


				len = APP_DIS_HARD_REV_STR_LEN;
				data = (uint8_t *)APP_DIS_HARD_REV_STR;
				Rop_Data_Send(HARD_REV,data,len);
				
				len = APP_DIS_SW_REV_STR_LEN;
				data = (uint8_t *)APP_DIS_SW_REV_STR;
				Rop_Data_Send(SW_REV,data,len);

			break;
		default:  
      break;
	}
}


