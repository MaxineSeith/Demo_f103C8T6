#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define SEND_BUF_SIZE  256         //发送

#define	USART1_ESP      ON
#define	USART2_ESP      OFF








void ESP8266_Init(void);
void ESP8266_Clear(void);
void ESP8266_SendData(unsigned char *data, unsigned short len);
unsigned char *ESP8266_GetIPD(unsigned short timeOut);
void UsartSendReceiveData(void);
void UsartReceive(void);
u16 ReceiveData_(unsigned char *dataPtr,const char *name);
void ESP8266_Sendmessage(uint16_t value);
/*ONENET*/
int ESP8266_MQTTSend(float value);
u16 ReceiveData_OneNet(unsigned char *dataPtr,const char *name);
void ESP8266_OneNet_Init(void);

//_Bool ESP8266_WaitRecive(uint16_t *received_len); 

_Bool ESP8266_WaitRecive(void);

int contains_substring(const u8 *main_string, const char *substring);
void ESP8266_Get_information(void);
int ESP8266_Get_data(const char *substring);
int ESP8266_Get_state(void);
int ESP8266_Get(const char *res1, const char *res2);

extern u8 water,fan;
#endif
