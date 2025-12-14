#include "main.h"
#include "esp8266.h"

#if (USART1_ESP)
    #define ESP8266_BUF         U1Buffer
    #define ESP8266_CNT         UART1_Rx_cnt
    #define ESP8266_BUFF_SIZE   USART_REC_LEN
    #define ESP8266_SEND(data, len)  USART1_send((unsigned char *)data, len)
#elif (USART2_ESP)
    #define ESP8266_BUF         U2Buffer
    #define ESP8266_CNT         UART2_Rx_cnt
    #define ESP8266_BUFF_SIZE   USART_REC_LEN
    #define ESP8266_SEND(data, len)  USART2_send((unsigned char *)data, len)
#else
    #error "Please define ESP8266_UART as USART1 or USART2!"
		#error "Please define ESP8266_UART as USART1 or USART2!"
#endif


unsigned short esp8266_cntPre = 0;
uint16_t timeCount1 = 0;
bool usart_send_flag=0;

extern u8 mode,warningFlag,load;
extern float voltage;
extern float volMax;
extern uint16_t current;
extern uint16_t currentMax;
//==========================================================
//	函数名称：	UsartSendReceiveData
//	函数功能：	串口1接收和发送数据
//	入口参数：	无
//	返回参数：	无
//	说明：		
//==========================================================
void UsartSendReceiveData(void)
{
	unsigned char *dataPtr = NULL;
	char SEND_BUF[250];
	int setValue = 0;

	dataPtr = ESP8266_GetIPD(0);

	timeCount1++;
	if (timeCount1 >= 5)
	{
		timeCount1 = 0;
		usart_send_flag = 1;
	}
	if (usart_send_flag == 1)
	{
		usart_send_flag = 0;

		memset(SEND_BUF, 0, sizeof(SEND_BUF)); // 清空缓冲区

		sprintf(SEND_BUF, "voltage%2d current%4d warningFlag%2d",(u16)(voltage*10),current,warningFlag);

		ESP8266_SendData((u8 *)SEND_BUF, strlen(SEND_BUF));
		
		ESP8266_Clear();
	}
}
void UsartReceive(void)
{
	unsigned char *dataPtr = NULL;
	char SEND_BUF[250];
	int setValue = 0;
		dataPtr = ESP8266_GetIPD(0);
		if (dataPtr != NULL)
	{
		setValue = ReceiveData_(dataPtr,"load:");
		if(setValue != NULL)
		{
			if(warningFlag == 0)
			{
				load = !load;
				if(load == 0)RELAY(OFF);
				else RELAY(ON);
			}
		}
		setValue = ReceiveData_(dataPtr,"volMax:");
		if(setValue != NULL)
		{
			volMax = (float)(setValue - 1)/10;
		}
				setValue = ReceiveData_(dataPtr,"currentMax:");
		if(setValue != NULL)
		{
			currentMax = setValue - 1;
		}
		
		ESP8266_Clear(); // 清空缓存
	}
}

//==========================================================
//	函数名称：	ESP8266_Clear
//	函数功能：	清空缓存
//	入口参数：	无
//	返回参数：	无
//	说明：	
//==========================================================
void ESP8266_Clear(void)
{
	memset(ESP8266_BUF, 0, sizeof(ESP8266_BUF));
	ESP8266_CNT = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//	函数功能：	等待接收完成
//	入口参数：	无
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{
	if(ESP8266_CNT == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(ESP8266_CNT == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		ESP8266_CNT = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = ESP8266_CNT;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//	函数功能：	发送命令
//	入口参数：	cmd：命令
//			 res：  需要检查的返回指令
//	返回参数：	0-成功	1-失败
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res, u16 time)
{	
  ESP8266_SEND((unsigned char *)cmd,strlen((const char *)cmd));
	
	while(time--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)ESP8266_BUF, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		delay_ms(1);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//	函数功能：	发送数据
//	入口参数：	data：数据
//			 len：  长度
//	返回参数：	无
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=0,%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">", 500))				//收到‘>’时可以发送数据
	{
			ESP8266_SEND(data , len);         //发送设备连接请求数据
	}
}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//	函数功能：	获取平台返回的数据
//	入口参数：	等待的时间(乘以10ms)
//	返回参数：	平台返回的原始数据
//	说明：		  不同网络设备返回的格式不同，需要去调试
//				      如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)ESP8266_BUF, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		delay_ms(1);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	ESP8266_Init
//	函数功能：	初始化ESP8266
//	入口参数：	无
//	返回参数：	无
//	说明：		
//==========================================================
void ESP8266_Init(void)
{	
	ESP8266_Clear();
	while(ESP8266_SendCmd("AT\r\n\r", "OK", 200))         //测试
	delay_ms(500);
	
	while(ESP8266_SendCmd("AT+CWMODE=2\r\n", "OK", 200))  //服务器搭建在WIFI模块上
	delay_ms(500);
	
	
	while(ESP8266_SendCmd("AT+CIPMUX=1\r\n", "OK", 200))  //启动多连接，建立服务器都需要配置
	delay_ms(500);
	
	while(ESP8266_SendCmd("AT+CIPSERVER=1,8080\r\n", "OK", 200))  //建立服务器
	delay_ms(500);
}
//==========================================================
//	函数名称：	ReceiveData_
//	函数功能：	处理WiFi端接收到的数据
//	入口参数：	无
//	返回参数：	接收到的数据值
//	说明：name：发送端键值
//==========================================================
u16 ReceiveData_(unsigned char *dataPtr,const char *name)
{
	char *str1 = 0, i;
	int setValue = 0;
	char setvalue[3] = {0};
	if(strstr((char *)dataPtr,name)!=NULL)
	{
		BEEP_DOING();
		str1 = strstr((char *)dataPtr,name);
		
		while (*str1 >= '0' && *str1 <= '9')
		{
			setvalue[i] = *str1;
			i++;
			str1++;
			if (*str1 == '\r')
				break;
			delay_ms(10);
		}
		setvalue[i] = '\0'; // 加上结尾符
		setValue = atoi(setvalue);
		if (setValue >= 0 && setValue <= 999)
		{
			return setValue + 1;
		}
		return NULL;
	}
	return NULL;
}



