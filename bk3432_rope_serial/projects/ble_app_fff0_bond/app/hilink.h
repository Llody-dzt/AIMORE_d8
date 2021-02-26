#ifndef _HILINK_H_
#define _HILINK_H_

#include "user_config.h"


extern unsigned char adc_item[3][4];//byte 0: adc voltage(unit 10mv), byte1: adc value high, byte2: adc value low 	

int hilink_init(void);


int hilink_deinit(void);

typedef int (*hilink_cb)(int cmd, int arg1, int arg2);

typedef int (*hilink_buf_cb)(char* buf, int l);

//Send command
int hilink_send(int cmd, int arg1, int arg2);
//Register recv callback
int hilink_recv(hilink_cb);


//0 not inited
//1 inited, no connection
//2 inited, connected
int hilink_status(void);
int hilink_status_isr(void (*cb)(int));


typedef int (*hilink_buf_cb)(char* buf, int l);
int hilink_send_buf(const char* s, int l);
int hilink_recv_buf(char* buf, int* l);

#ifdef LLODY
extern char verbuf2[10];
extern void UartCommandDispatch(unsigned char *pBuff, int len);
#endif
void d8GpioInit();
void NumofTurns();
#endif


