#ifndef USER_QUEUE_H
#define USER_QUEUE_H

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

typedef struct User_Queue
{
	uint8* buf;
	uint16 max_len; 
	uint16 front;
	uint16 rear;
}USER_QUEUE_T;

void User_Queue_Init(USER_QUEUE_T* queue,uint8* pbuf,uint16 max_len);
void User_Queue_Clear(USER_QUEUE_T* queue);
uint16 User_Queue_Get_Len(USER_QUEUE_T* queue);
uint16 User_Queue_Get_spare(USER_QUEUE_T* queue);
uint8 User_Queue_Data_In(USER_QUEUE_T* queue,uint8* data,uint16 len);
uint8 User_Queue_Data_Out(USER_QUEUE_T* queue,uint8* buf,uint16 len);
uint8 User_Queue_Data_Copy(USER_QUEUE_T* queue,uint8* buf,uint16 len);
uint8 User_Queue_Data_Trans(USER_QUEUE_T* queue_d,USER_QUEUE_T* queue_s,uint16 len);
uint8 User_Queue_Data_Del(USER_QUEUE_T* queue,uint16 len);
uint8 User_Queue_Is_Full(USER_QUEUE_T* queue);
uint8 User_Queue_Is_Empty(USER_QUEUE_T* queue);

#endif



