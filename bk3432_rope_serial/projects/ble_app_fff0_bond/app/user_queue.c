#include "user_queue.h"
#include "string.h"

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

void User_Queue_Init(USER_QUEUE_T* queue,uint8* pbuf,uint16 max_len)
{
	queue->buf = pbuf;
	queue->max_len = max_len;
	queue->front = queue->rear = 0x00;
}

void User_Queue_Clear(USER_QUEUE_T* queue)
{
	queue->front = queue->rear = 0x00;
}

uint16 User_Queue_Get_Len(USER_QUEUE_T* queue)
{
	return (queue->max_len + queue->rear - queue->front) % queue->max_len; 
}
uint16 User_Queue_Get_spare(USER_QUEUE_T* queue)
{
	return queue->max_len - ((queue->max_len + queue->rear - queue->front) % queue->max_len) - 1;
}
uint8 User_Queue_Data_In(USER_QUEUE_T* queue,uint8* data,uint16 len)
{
	if(User_Queue_Get_spare(queue) >= len)
	{
			uint16 i;
			for(i=0;i<len;i++)
			{
				queue->buf[queue->rear] = data[i];
				queue->rear = ((queue->rear + 1) % queue->max_len);
			}
			return 0x00;		
	}
	else
	{
		return 0x01;
	}
}
uint8 User_Queue_Data_Out(USER_QUEUE_T* queue,uint8* buf,uint16 len)
{
	if(User_Queue_Get_Len(queue) >= len)
	{
		uint16 i;
		for(i=0;i<len;i++)
		{
			buf[i] = queue->buf[queue->front];
			queue->front = ((queue->front + 1) % queue->max_len);
		}
		return 0x00;
	}
	else
	{
		return 0x01;
	}
}
uint8 User_Queue_Data_Copy(USER_QUEUE_T* queue,uint8* buf,uint16 len)
{
	if(User_Queue_Get_Len(queue) >= len)
	{
		uint16 i;
		for(i=0;i<len;i++)
		{
			buf[i] = queue->buf[(queue->front+i)%queue->max_len];
		}
		return 0x00;
	}
	else
	{
		return 0x01;
	}
}
uint8 User_Queue_Data_Trans(USER_QUEUE_T* queue_d,USER_QUEUE_T* queue_s,uint16 len)
{
	if((User_Queue_Get_Len(queue_s) >= len) && (User_Queue_Get_spare(queue_d) >= len))
	{
		uint16 i;
		for(i=0;i<len;i++)
		{
			queue_d->buf[queue_d->rear] = queue_s->buf[queue_s->front];
			queue_s->front = ((queue_s->front + 1) % queue_s->max_len);
			queue_d->rear = ((queue_d->rear + 1) % queue_d->max_len);
		}
		return 0x00;
	}
	else
	{
		return 0x01;
	}
}
uint8 User_Queue_Data_Del(USER_QUEUE_T* queue,uint16 len)
{
	if(User_Queue_Get_Len(queue) >= len)
	{
		queue->front = ((queue->front + len) % queue->max_len);

		return 0x00;
	}
	else
	{
		return 0x01;
	}
}
uint8 User_Queue_Is_Full(USER_QUEUE_T* queue)
{
	if((queue->rear + 1)% queue->max_len == queue->front)
	{
		return 0x01;
	}
	else
	{
		return 0x00;
	}
}
uint8 User_Queue_Is_Empty(USER_QUEUE_T* queue)
{
	if(queue->front == queue->rear)
	{
		return 0x01;
	}
	else
	{
		return 0x00;
	}
}




