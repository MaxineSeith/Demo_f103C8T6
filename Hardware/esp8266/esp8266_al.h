#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define SEND_BUF_SIZE  64         //发送
#define ESP8266_UART	USART1_ESP	//确定分配串口





#if (ESP8266_UART == USART1_ESP)
    #define ESP8266_BUF         				U1Buffer
    #define ESP8266_CNT         				UART1_Rx_cnt
    #define ESP8266_BUFF_SIZE   				USART_REC_LEN
    #define ESP8266_SEND(data, len)  		USART1_send((unsigned char *)data, len)
#elif (ESP8266_UART == USART2)
    #define ESP8266_BUF         				U2Buffer
    #define ESP8266_CNT         				UART2_Rx_cnt
    #define ESP8266_BUFF_SIZE   				USART_REC_LEN
    #define ESP8266_SEND(data, len)  		USART2_send((unsigned char *)data, len)
#else
    #error "Please define ESP8266_UART as USART1 or USART2!"
		#error "Please define ESP8266_UART as USART1 or USART2!"
#endif


void UsartSendReceiveData(void);   //串口发送接收数据	
_Bool ESP_WaitRecive(void);	
_Bool ESP8266_SendCmd(char *cmd, char *res, u16 time);
void Esp_Init(int screen)/* screen屏幕标识标志：OLED--0，LCD--1 */;
void ESP8266_Clear(void);
char Esp_Link_Init(void);
u16 ReceiveData(const char *name);
		


#endif
