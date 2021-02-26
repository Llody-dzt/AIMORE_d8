#include "hilink.h"

#include "arch.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include <string.h>

#include "boot.h"      // boot definition

#include "flash.h"
#include "uart.h"      	// UART initialization
#include "uart2.h"      	// UART2 initialization
#include "gpio.h"

#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
#include "rf.h"        // RF initialization
#endif // BLE_EMB_PRESENT || BT_EMB_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"       // application functions
#endif // BLE_APP_PRESENT

#include "reg_assert_mgr.h"
#include "app_task.h"
#include "oads.h"
#include "app_fff0.h"  
#include "pwm.h"
#include "adc.h"
#include "rwip.h"      // RW SW initialization

#ifdef LLODY
	#include "ftmps_task.h"
	void UartCommandDispatch(unsigned char *pBuff, int len);
#endif

#ifdef D8LLODY
#include "adc.h"
struct D8
{
	uint32_t Gers;  //??????
	uint32_t NTurns;   //?????��
};
struct D8 d8;

int D8count=100;
void d8GpioInit();
void GetD8Gears(uint16_t d8ADC);

#endif

void hlink_set_status(uint8_t statu);

extern void app_fff1_send_lvl(uint8_t* buf, uint8_t len);

uint8_t auth_flag = 1;
void (*ble_status_cb)(int) = NULL;
hilink_cb g_cmd_recv_cb = NULL;

unsigned char adc_item[3][4];//byte 0: adc voltage(unit 10mv), byte1: adc value high, byte2: adc value low

#define MAC_STR_FMT	"%2x:%2x:%2x:%2x:%2x:%2x"
#define HILINK_FLASH_ADDR	(0x8000)
#define AUTH_LEN 11
typedef struct {
	uint8_t addr[6];
	char ble_name[32];
	uint8_t version[16];
	uint8_t uuid[10];
	uint8_t company_name[16];
	uint8_t product_id[16];
	uint8_t adc_item1[4];//byte 0: adc voltage(unit 10mv), byte1: adc value high, byte2: adc value low 
	uint8_t adc_item2[4];
	uint8_t adc_item3[4];	
	uint8_t baudrate[4];
}hilink_flash;

#define U_LEN	(128)
uint8_t u_buf[U_LEN];
uint16_t u_len = 0;
uint8_t hilink_pmt[U_LEN+8] = {0};

/*from arch_main*/
static void bdaddr_env_init(void)
{
	struct bd_addr co_bdaddr;
	struct bd_addr abc={0xFF, 0xFE, 0x41, 0x42, 0x43, 0x37};
	flash_read(FLASH_SPACE_TYPE_MAIN, 0x27ff0/4, 6, &co_bdaddr.addr[0]);
	if(co_bdaddr.addr[0]!=0xff ||co_bdaddr.addr[1]!=0xff||
	        co_bdaddr.addr[2]!=0xff||co_bdaddr.addr[3]!=0xff||
	        co_bdaddr.addr[4]!=0xff||co_bdaddr.addr[5]!=0xff )
	{
		memcpy(&co_default_bdaddr,&co_bdaddr,6);
	}else{
		int swp = 0;
		for(int i = 0; i < 3; ++i){
			swp = abc.addr[5 - i];
			abc.addr[5 - i] = abc.addr[i];
			abc.addr[i] = swp;
		}
		memcpy(&co_default_bdaddr,&abc,6);
	}
}

static void ble_clk_enable(void)
{
	REG_AHB0_ICU_BLECLKCON =  0;
}
/*from arch_main*/
void uart_printf_buf(uint8_t *buf, uint8_t len)
{
	uart_printf("hex:");
	for(int i = 0; i < len; ++i){
		uart_printf("%02x ", buf[i]);
	}
	uart_printf("\r\n");
}
/* for uart cmd */
#define RLEN	256
uint8_t r_buf[RLEN] = {0};

//uint8_t scan_tick = 0;
uint64_t r_set = 0;
uint64_t r_get = 0;
typedef enum {
	DEINIT, //0 not inited
	INIT,//1 inited, no connection
	CONNECT//2 inited, connected
}STATUS;
STATUS status = DEINIT;

static uint8_t gpio_list[] = {0x31, 0x32, 0x35, 0x14, 0x17, 0x16, 0x34, 0x33, 0x13, 0x12, 0x11, 0x10, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02};

static int save_flash_data(uint8_t *buf, uint8_t len)
{
	flash_erase(FLASH_SPACE_TYPE_NVR, HILINK_FLASH_ADDR, len);
	flash_write(FLASH_SPACE_TYPE_NVR, HILINK_FLASH_ADDR, len, buf);
	return 0;
}

uint64_t count_ms = 0;

uint8_t xor_get(uint8_t *buf, uint8_t len){
	uint8_t xor = 0x99;
	int i = 0; 
	for(i = 0; i < len; ++i){
		xor ^= buf[i];
	}
	return xor;
}
uint16_t hi_crc16(uint8_t *buf, uint8_t len){
	unsigned short wCRCin = 0xFAAA;
	unsigned short wCPoly = 0x1888;
	
	while (len--) 	
	{
		wCRCin ^= *(buf++) << 8;
		for(int i = 0;i < 8;i++)
		{
			if(wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}

uint8_t xor_crc16(uint8_t *buf, uint8_t len){
	if(len < 3)return -1;
	if(buf[len - 3] != xor_get(buf, len - 3))return -1;
	if((buf[len - 2] * 256 + buf[len -1]) != hi_crc16(buf, len - 2))return -1;

	return 0;
}

static int check_flash_data_valid(uint8_t *buf, uint8_t len){
	if(xor_crc16(buf, len))return -1;
	hilink_flash *p = (hilink_flash *)buf;
	if(strlen(p->ble_name) < 10 || strlen(p->ble_name) > 28)return -1;
	return 0;
}

uint8_t check_pwd(uint8_t *buf, uint8_t len){
	if(len != AUTH_LEN)return -1;
	
	return xor_crc16(buf, len);
}

void flash_get(void){
	uint8_t fdata[U_LEN] = {0};
	flash_read(FLASH_SPACE_TYPE_NVR, HILINK_FLASH_ADDR, sizeof(fdata), (uint8_t *)&fdata);
	uart_send(fdata, sizeof(fdata));
}

void flash_set(uint8_t *buf, uint8_t len){
	if(len != U_LEN){
		uart_printf("len error\r\n");
		return;
	}
	if(xor_crc16(buf, len)){
		uart_printf("xor_crc16 error\r\n");
		return;
	}
	if(check_flash_data_valid(buf, len)){
		uart_printf("flash pmt error\r\n");
		return;
	}
	save_flash_data(buf, len);
	uart_printf("flash set ok\r\n");
	return;	
}
uint8_t check_gpio(uint8_t pin){
	for(int i = 0; i < sizeof(gpio_list); ++i){
		if(pin == gpio_list[i])return 0;
	}
	return -1;
}
void hi_gpio_get(uint8_t pin){
	if(check_gpio(pin))uart_printf("gpio pin invalid\r\n");
	gpio_config(pin, INPUT, PULL_NONE);
	uart_printf("uart pin(%x) get(%d)\r\n", pin, gpio_get_input(pin));
}

void hi_gpio_set(uint8_t pin, uint8_t level){
	if(check_gpio(pin))uart_printf("gpio pin invalid\r\n");
	gpio_config(pin, OUTPUT, PULL_NONE);
	gpio_set(pin, level? 1 : 0);
	uart_printf("uart pin(%x) set(%d) ok\r\n", pin, level);
}

void hi_adc_get(uint8_t channel){
	if(channel > 7)uart_printf("adc channel invalid\r\n");
	adc_init(channel, 1);
	
//	uart_printf("uart adc(%d) get(%d) ok", channel, adc_get_value());
}
extern void sys_Reset(void);
void hi_reset(void){
	uart_printf("Hi reset\r\n");
	sys_Reset();
}

void uart_handle(uint8_t *buf, uint8_t len){
	//uart_printf_buf(buf, len);
	static uint8_t flash_ready = 0;
	if(flash_ready){
		flash_ready = 0;
		flash_set(buf, len);
		return;
	}
	if(!memcmp("Hi flash get", buf, strlen("Hi flash get"))){
		uart_printf("flash data:");
		flash_get();
	}else if(!memcmp("Hi flash set", buf, strlen("Hi flash set"))){
		flash_ready = 1;
		uart_printf("ready to set flash\r\n");
	}else if(!memcmp("Hi gpio get", buf, strlen("Hi gpio get"))){
		hi_gpio_get(buf[strlen("Hi gpio get")]);
	}else if(!memcmp("Hi gpio set", buf, strlen("Hi gpio set"))){
		hi_gpio_set(buf[strlen("Hi gpio set")], buf[strlen("Hi gpio set") + 1]);
	}else if(!memcmp("Hi adc get", buf, strlen("Hi adc get"))){
		hi_adc_get(buf[strlen("Hi adc get")]);
	}else if(!memcmp("Hi reset", buf, strlen("Hi reset"))){
		hi_reset();
	}else{
		uart_printf("pmt error\r\n");
	}
}

treadmill_Data_t D8data;		
uint16_t check_adc[5]={0};
uint16_t adcCheck=0;
int adcNum=0;
uint16_t adcCBuf[8]={0};
uint16_t abadc;
uint16_t aaaadc[9];
uint16_t aaaadc1[9];
uint16_t hget(uint16_t *buf, uint16_t len)
{
	uint16_t a, b;
	for(int i = 0; i < len; i++){
		a = 0;
		b = 0;
		for(int j = 0; j < len; j++){
			if(buf[i] < buf[j]){
				a++;
			}else if(buf[i] == buf[j]){
				b++;
			}
		}
		if(a <= len/2 && a+b >= len/2){
			return buf[i];
		}
	}
}

uint32_t e_time = 0;
uint32_t s_time = 0;
uint8_t bike_status = 0;

#if 1
uint32_t r_time=0;  //running time
uint32_t NTurns5=0;
#endif

void instantaneous_Power()
{
	switch(d8.Gers)
	{
		case 1:
			D8data.instantaneousPower=16*D8data.instantaneousCadence/10-28;
		break;	
		
		case 2:
			D8data.instantaneousPower=23*D8data.instantaneousCadence/10-37;
		break;
		
		case 3:
			D8data.instantaneousPower=28*D8data.instantaneousCadence/10-47;
		break;
		
		case 4:
			D8data.instantaneousPower=32*D8data.instantaneousCadence/10-50;
		break;
	
		case 5:
			D8data.instantaneousPower=37*D8data.instantaneousCadence/10-57;
		break;
	
		case 6:
			D8data.instantaneousPower=45*D8data.instantaneousCadence/10-69;
		break;
		
		case 7:
			D8data.instantaneousPower=52*D8data.instantaneousCadence/10-75;
		break;
		
		case 8:
			D8data.instantaneousPower=62*D8data.instantaneousCadence/10-89;
		break;
	}
}
void average_Power()
{
	switch(d8.Gers)
	{
		case 1:
			D8data.averagePower=16*D8data.averageCadence/10-28;
		break;	
		
		case 2:
			D8data.averagePower=23*D8data.averageCadence/10-37;
		break;
		
		case 3:
			D8data.averagePower=28*D8data.averageCadence/10-47;
		break;
		
		case 4:
			D8data.averagePower=32*D8data.averageCadence/10-50;
		break;
	
		case 5:
			D8data.averagePower=37*D8data.averageCadence/10-57;
		break;
	
		case 6:
			D8data.averagePower=45*D8data.averageCadence/10-69;
		break;
		
		case 7:
			D8data.averagePower=52*D8data.averageCadence/10-75;
		break;
		
		case 8:
			D8data.averagePower=62*D8data.averageCadence/10-89;
		break;
	}
}


void hi_timer_cb()
{
	count_ms++;
	static uint8_t initd = 1;
	if(count_ms > 6 && u_len){
		uart_handle(u_buf, u_len);
		u_len = 0;
		memset(u_buf, 0, U_LEN);
	}
	
	if(count_ms%10 == 0){

		hi_adc_get(5);
		uint16_t adcc00 = adc_get_value();
		aaaadc1[(count_ms/10)%9] = adcc00;	
		abadc=hget(aaaadc1, 9);
		
		hi_adc_get(1);
		uint16_t adcc = adc_get_value();
		aaaadc[(count_ms/10)%9] = adcc;
		GetD8Gears(hget(aaaadc, 9));
		
		//uart_printf("ADC1-%d  ADC2-%d\r\n",abadc, hget(aaaadc, 9));
	}
			
	if(ke_state_get(TASK_APP) == APPM_CONNECTED)
		hlink_set_status(CONNECT);
	else 
		hlink_set_status(INIT);
	
#ifdef D8LLODY
		
  average_Power();
	instantaneous_Power();
	
	if(bike_status){
		s_time++;
		if(bike_status != 2)
		{
			e_time++;
			
			#if 1
			if(r_time < 500)
			{
				r_time++;
				NTurns5++;
			}
			else
			{
				r_time=0;
				NTurns5=0;
			}
			#endif
		}
			
		
	}
	
	if(D8count<1)
	{
		#if 1	
		uint32_t a1=d8.NTurns*1000;
		
		D8data.elapsedTime = e_time/100;
		D8data.resistanceLevel = d8.Gers;    			
		D8data.averageCadence= d8.NTurns *60 / D8data.elapsedTime;		
		D8data.instantaneousCadence = (NTurns5*12) / 2;				
		D8data.averageSpeed= (D8data.averageCadence * 700 / 60);			
		D8data.instantaneousSpeed = (D8data.instantaneousCadence *700 / 6 );		
		D8data.totalDistance = a1 *7 / 3600 ;		
		D8data.totalEnergy = a1 * 27 / 60000;					
		D8data.energyPerHour = D8data.totalEnergy / D8data.elapsedTime / 3600;
		D8data.energyPerMinute = D8data.totalEnergy / D8data.elapsedTime /60;				
		#endif
		
		if(s_time > 500 && bike_status ==1){			
				D8data.elapsedTime = e_time/100;
				
				D8data.flags=0x0000| (1 << 5)| (1 << 6) | (1 << 7)  |  (1 << 8)|  (1 << 11); 
				TreadmillDataNotify(D8data);	
				D8data.flags=0x0000|(1 << 1) | (1 << 2) | (1 << 3) | (1 << 4); 
				TreadmillDataNotify(D8data);
				
				TrainingStatusRead(quick_start);
				FitnessMachineStatusnotify(pause_by_user, NULL, 0);	
				bike_status = 2;
		}
		
		else if(s_time > 6500){//stop
			bike_status = 0;
			s_time = 0;	
			d8.NTurns = 0;
			e_time = 0;			
			D8data.flags=0x0000| (1 << 5)| (1 << 6) | (1 << 7)  |  (1 << 8)|  (1 << 11); 
			TreadmillDataNotify(D8data);	
			D8data.flags=0x0000|(1 << 1) | (1 << 2) | (1 << 3) | (1 << 4); 
			TreadmillDataNotify(D8data);
			
			TrainingStatusRead(idle);
			FitnessMachineStatusnotify(stop_by_user, NULL, 0);
			
		}else{	
			if(bike_status)	{	
				
				D8data.flags=0x0000| (1 << 5)| (1 << 6) | (1 << 7)  |  (1 << 8)|  (1 << 11); 
				TreadmillDataNotify(D8data);	
				
				D8data.flags=0x0000|(1 << 1)| (1 << 2)|(1 << 3)|(1 << 4); 
				TreadmillDataNotify(D8data);	
				
				TrainingStatusRead(quick_start);
				FitnessMachineStatusnotify(start_by_user, NULL, 0);	
			}
		}	
		D8count=100;		
	}
	D8count--;
#endif	
}


void hilink_timer_init(void){
	PWM_DRV_DESC timer_desc;

	timer_desc.channel = 1;            				  
    timer_desc.mode    = 1<<0 | 1<<1 | 1<<2 | 0<<4; 
	timer_desc.pre_divid = 100;
    timer_desc.end_value  = 10000;                      
    timer_desc.duty_cycle = 0;                        
    timer_desc.p_Int_Handler = hi_timer_cb;  	

	REG_AHB0_ICU_PWMCLKCON |= (1<<1);
    REG_AHB0_ICU_PWMCLKCON &= ~(7<<12);
    REG_AHB0_ICU_PWMCLKCON |= (8<<12);
	pwm_init(&timer_desc);
}

static void uart_rx_handler(uint8_t *buf, uint8_t len)
{
	static uint8_t auth = 1;
	
	#if 0	
		if(status == CONNECT){
			extern void app_fff1_send_lvl(uint8_t* buf, uint8_t len);
			app_fff1_send_lvl(buf, len > 128 ? 128 : len);

			#if 0
				uint8_t	pbuf[] ={0xBB,0x02,0x58,0x02,0x08,0x01,0x14,0x01,0x14,0xfe,0xfe,0x00,0x00};
				UartCommandDispatch(pbuf,strlen(pbuf));	
			#else
				UartCommandDispatch(buf,len);	
			#endif			
		}					
	#endif
	
	if(len == AUTH_LEN && auth){
		auth = check_pwd(buf, len);
		if(!auth){
			UART_PRINTF("Hilink Auth OK\r\n");
			//hilink_timer_init();
			return;
		}
	}
	if(auth)return;
	else count_ms = 0;
	len = u_len + len > U_LEN ? 0 : len;
	if(len){
		memcpy(u_buf + u_len, buf, len);
		u_len += len;
	}
}
/* for uart cmd */

/////////
void hilink_set_r_buf(const char* s, int l){
	for(int i = 0; i < l; i++){
		r_buf[r_set++%RLEN] = s[i];
	}	
}

void hilink_get_r_buf(char* s, int *l){
	r_get = r_get + RLEN < r_set ? (r_set - RLEN) : r_get;
	*l = r_get + *l > r_set ? (r_set - r_get) : *l;
	for(int i = 0; i < *l; i++){
		s[i] = r_buf[r_get++%RLEN];
	}	
}

hilink_buf_cb hilink_bt_cb = NULL;
int hilink_send_buf(const char* s, int l) {
		app_fff1_send_lvl((uint8_t*)s, l > 128 ? 128 : l);
		return 0;
}

int hilink_send(int cmd, int arg1, int arg2)
{
	uint8_t send_buffer[12];
	int *p = (int *)send_buffer;	
	p[0] = cmd;
	p[1] = arg1;
	p[2] = arg2;
	app_fff1_send_lvl(send_buffer, sizeof(send_buffer));
	
	return 0;
}

int hilink_recv_buf_isr(hilink_buf_cb cb) {
	if(cb != NULL){
		hilink_bt_cb = cb;
		return 0;
	}else{
		return -1;
	}		
}

int hilink_recv_isr(hilink_cb cb)
{
	if(cb != NULL)
	{
		g_cmd_recv_cb = cb;
		return 0;
	}else{
		return -1;
	}	
}

int hilink_recv_buf(char* buf, int* l){
	hilink_get_r_buf(buf, l);
	return 0;
}

void hlink_set_status(uint8_t statu){
	if(status == statu)
		return;
		
	status =(STATUS)( statu > CONNECT ? CONNECT : statu);
	if(ble_status_cb != NULL){
		ble_status_cb(status);
	}
	if(statu == CONNECT){
		gpio_config(0x34, OUTPUT, PULL_NONE);
		gpio_set(0x34, 1);
		
		TrainingStatusRead(quick_start);	
		FitnessMachineStatusnotify(start_by_user,0,0);
		// TrainingStatusRead(quick_start);
		// FitnessMachineStatusnotify(start_by_user, NULL, 0);
	}else{
		gpio_config(0x34, OUTPUT, PULL_NONE);
		gpio_set(0x34, 0);
	}
}

//get in app_task.c
int hilink_status(){
	return status;
}

int hilink_status_isr(void (*cb)(int)){
	if(cb != NULL)
	{
		ble_status_cb = cb;
		return 0;
	}
	else
	{
		return -1;
	}
}

int hilink_deinit(){
	hlink_set_status(DEINIT);
	return 0;
}

char *hi_get_dev_name(){
	return ((hilink_flash *)(&hilink_pmt[0]))->ble_name;
}

static void uart2_rx_handler(uint8_t *buf, uint8_t len)
{
	if(status == CONNECT){
		// uart_printf("uart2: ");
		// for(uint8_t i=0; i<len; i++)
		// {
		// 	UART_PRINTF("0x%x ", buf[i]);
		// }
		// uart_printf("\r\n");
		extern void app_fff1_send_lvl(uint8_t* buf, uint8_t len);
		app_fff1_send_lvl(buf, len > 128 ? 128 : len);
		//gatt_data_set(buf, len);
		#ifdef LLODY
		UartCommandDispatch(buf,strlen(buf));
		#endif
	}
}

uint32_t get_int4(uint8_t *s, uint8_t len){
	if(len > 4)return 0;
	uint32_t result = 0;
	for(int i = 0; i < len; ++i){
		result |= s[i] << ((len - i - 1)*8);
	}
	return result;
}

void hilink_pmt_init(hilink_flash *p)
{
	struct bd_addr abc;
	memcpy(&abc,p->addr,6);
	int swp = 0;
	for(int i = 0; i < 3; ++i){
		swp = abc.addr[5 - i];
		abc.addr[5 - i] = abc.addr[i];
		abc.addr[i] = swp;
	}
	memcpy(&co_default_bdaddr, &abc, 6);
	memcpy(adc_item[0], p->adc_item1, 3);
	memcpy(adc_item[1], p->adc_item2, 3);
	memcpy(adc_item[2], p->adc_item3, 3);
	#if UART_CHANGE
	uart_init(get_int4(&p->baudrate[0], 4));
	uart_cb_register(uart2_rx_handler);
	#else
		uart2_init(115200);
		uart2_cb_register(uart2_rx_handler);
	#endif
}

int hi_handle_default(char *buf, int l){
	uart_printf_buf(buf, l);
	uart2_send(buf, l);
}


void user_pwm_init(void)
{
//	rwip_prevent_sleep_set(BK_DRIVER_TIMER_ACTIVE);
//		
//	PWM_DRV_DESC Led_desc;
//	Led_desc.channel=0;
//	Led_desc.mode=0X01;
//	pwm_init(&Led_desc);
}

int hilink_init(void)
{
#if UART_CHANGE
	uart2_init(115200);
	uart2_cb_register(uart_rx_handler);
#else
	uart_init(115200);
	uart_cb_register(uart_rx_handler);
#endif
	hilink_recv_buf_isr(hi_handle_default);
	hlink_set_status(INIT);
	
	icu_set_sleep_mode(1);
	rwip_prevent_sleep_set(BK_DRIVER_TIMER_ACTIVE);

	flash_read(FLASH_SPACE_TYPE_NVR, HILINK_FLASH_ADDR, sizeof(hilink_pmt), (uint8_t *)&hilink_pmt);
	if(0)//((!check_flash_data_valid(hilink_pmt, sizeof(hilink_pmt))))
	{
		hilink_flash *p = (hilink_flash *)(&hilink_pmt);
		hilink_pmt_init(p);
		UART_PRINTF("mac=%02X:%02X:%02X:%02X:%02X:%02X\r\n", p->addr[0], p->addr[1], \
		 							p->addr[2], p->addr[3], p->addr[4], p->addr[5]);
		// UART_PRINTF("uuid=%s\r\n", data.uuid);
		UART_PRINTF("ble_name=%s\r\n", p->ble_name);
		UART_PRINTF("baudrate=%d\r\n", get_int4(&p->baudrate[0], 4));
		// UART_PRINTF("ch1=%d ch7=%d\r\n", data.adc_calib_gnd, data.adc_calib_ch7);
	}
	else
	{
		memset(hilink_pmt, 0, 128);
		memcpy(((hilink_flash *)(&hilink_pmt[0]))->ble_name, "Hi-TEST-1FFFFXXXXX", strlen("Hi-TEST-1FFFFXXXXX"));
		bdaddr_env_init();
	}

	ble_clk_enable();
	rwip_init(0);
	
	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 15); //BLE INT
	REG_AHB0_ICU_IRQ_ENABLE = 0x03;
	
	#ifdef D8LLODY  

	 d8GpioInit();

	//	D8data.flags=treadmill_total_distance|inclination_and_ramp_angle_setting|treadmill_expended_energy|treadmill_elapsed_time|instantaneous_pace;	
	
	//	D8data.flags=treadmill_total_distance|inclination_and_ramp_angle_setting|treadmill_expended_energy|treadmill_elapsed_time|Instantaneous_Cadence|Average_Cadence|treadmill_average_speed;
		d8.Gers =0;
		d8.NTurns=0;
	#endif
	
	user_pwm_init();

	return 0;

}

#ifdef LLODY
char verbuf2[10]={0};
void UartCommandDispatch(unsigned char *pBuff, int len)
{
    // treadmill_Data_t data;
	// 	uint8_t send[3];
	//   data.flags=treadmill_elapsed_time|treadmill_total_distance|treadmill_expended_energy|heart_rate|instantaneous_speed;

	// 	switch(pBuff[0])
	// 	{
	// 		case 0xBB:  //��y?Y��?2?
	// 		{			
	// 			data.elapsedTime = (pBuff[1]<<8) + pBuff[2];				
	// 			data.totalDistance = (pBuff[3]<<8) + pBuff[4];					
	// 			data.totalEnergy = (pBuff[5]<<8) + pBuff[6];			
	// 			data.heartRate= pBuff[9];
	// 			data.instantaneousSpeed = pBuff[10]*10;	
				
	// 			TreadmillDataNotify(data);	
				
	// 			if( pBuff[10] > 0 )
	// 			{
	// 				TrainingStatusRead(quick_start);	
	// 				FitnessMachineStatusnotify(start_by_user,&pBuff[10],0x01);
	// 			}
	// 			else
	// 			{
	// 				TrainingStatusRead(idle);	
	// 				FitnessMachineStatusnotify(stop_by_user,0,0x01);
	// 			}
				
	// 		}
	// 		break;	
			
	// 		case 0xE4: //2��?��?����D���䨬? 
	// 		{			
	// 			if(pBuff[2] == 0x01) 
	// 			{			
	// 					TrainingStatusRead(quick_start);
	// 					FitnessMachineStatusnotify(start_by_user,NULL,0x00);		
	// 			}					
	// 			else	
	// 			{
	// 					TrainingStatusRead(idle);
	// 					FitnessMachineStatusnotify(stop_by_user,NULL,0x00);			
	// 			}					
	// 		}
	// 		break;
				
	// 		case 0x09:
	// 		{				
	// 			char buf[10]={0};		
	// 			sprintf(buf,"%d.%d.%d",pBuff[2],pBuff[3],pBuff[4]);
	// 			strncpy(verbuf2,buf,strlen(buf));				
	// 		}
	// 		break;
						
	// 		default:
	// 		break;
	// }
}
#endif

#ifdef D8LLODY

#define KEYBOARD_MAX_ROW_SIZE 5
#define KEYBOARD_MAX_COL_SIZE 3

uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOB_0,GPIOB_1,GPIOB_2,GPIOB_3,GPIOB_4}; //P1.0,p1.1,p1.2,p1.3,p1.4
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOD_1,GPIOD_2,GPIOD_5}; //p3.1,p3.2,p3.5

void d8_wakeup_config(void)
{
	REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	//Delay_ms(2);
	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
  REG_AHB0_ICU_INT_ENABLE |= (0x01 << 9);
}

void user_gpio_isr()
{
	d8.NTurns++;	
	NTurns5++;
	bike_status = 1;
	s_time = 0;
	//UART_PRINTF("The current number of turns %d\r\n", d8.NTurns);
	d8_wakeup_config();	
}

void d8GpioInit()
{	
	gpio_config(BlueLedPort,INPUT,PULL_HIGH); 
	gpio_cb_register(user_gpio_isr);
	
	REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	Delay_ms(2);
	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(BlueLedPort>>4)+(BlueLedPort&0x0f));
	
	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 9);
}
void GetD8Gears(uint16_t d8ADC)
{		
	uint32_t d8_GER=0;
	uint32_t d8_GER1=0;
	uint32_t d8_GER2=0;
	
	d8_GER=abadc*470;   //272
	d8_GER=d8_GER/d8ADC;
	d8_GER=(d8_GER-68)/10;
	
	uart_printf("(%d)\r\n", d8_GER);
	if(d8_GER >= 67)  d8.Gers=8;
	else if(d8_GER >= 60 && d8_GER < 67)  d8.Gers=7;	
	else if(d8_GER >= 52 && d8_GER < 60)  d8.Gers=6;
	else if(d8_GER >= 44 && d8_GER < 52)  d8.Gers=5;
	else if(d8_GER >= 35 && d8_GER < 44)  d8.Gers=4;
	else if(d8_GER >= 25 && d8_GER < 35)  d8.Gers=3;	
	else if(d8_GER >= 19 && d8_GER < 25)  d8.Gers=2;	
	else if( d8_GER < 19)  d8.Gers=1;
		
	// D8data.elapsedTime=600;
	// D8data.totalEnergy=300;
}
//void GetD8Gears(uint16_t d8ADC)
//{		
//	if(d8ADC >= 0xF5)  d8.Gers=1;
//	else if(d8ADC >= 0x9F && d8ADC <= 0xb3)  d8.Gers=2;	
//	else if(d8ADC >= 0x75 && d8ADC <= 0x89)  d8.Gers=3;
//	else if(d8ADC >= 0x5D && d8ADC <= 0x71)  d8.Gers=4;
//	else if(d8ADC >= 0x54 && d8ADC <= 0x5C)  d8.Gers=5;
//	else if(d8ADC >= 0x47 && d8ADC <= 0x53)  d8.Gers=6;	
//	else if(d8ADC >= 0x42 && d8ADC <= 0x46)  d8.Gers=7;	
//	else if( d8ADC <= 0x3F)  d8.Gers=8;
//	
//	D8data.instantaneousSpeed=d8ADC;
//	D8data.rampAngle = d8.Gers;
//	D8data.totalDistance = d8.NTurns;	
//}
#endif





