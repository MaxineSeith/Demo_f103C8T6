#include "main.h"
#include "esp8266.h"

////////////////////////只要修改三要素/////////////////////////////////
#define PRODUCEKEY 			"2bWg8Gc7k1"//产品ID
#define DEVICENAME 			"ESP8266TEST"			//设备名称
#define TOKEN       		"version=2018-10-31&res=products%2F2bWg8Gc7k1%2Fdevices%2FESP8266TEST&et=1780553169&method=md5&sign=%2BTz%2BnruiiY8WrcHIqWusIw%3D%3D" //token
///////////////////////////////////////////////////////////////////////
static const char MQTT_CONFIG_CMD[] = "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n";
static const char MQTT_SUBSCRIBE_CMD[] = "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/%s\",%d\r\n";

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
_Bool ESP8266_WaitRecive(uint16_t *received_len) {
    if(ESP8266_CNT == 0) {
        *received_len = 0;  // 无数据时返回0
        return REV_WAIT;
    }
    
    if(ESP8266_CNT == esp8266_cntPre) {
        *received_len = ESP8266_CNT;  // 返回实际接收的字节数
        ESP8266_CNT = 0;              // 维持原有清空计数逻辑
        esp8266_cntPre = 0;           // 同时清空预存值（新增）
        return REV_OK;
    }
    
    esp8266_cntPre = ESP8266_CNT;
    *received_len = 0;  // 未完成接收时返回0
    return REV_WAIT;
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
    ESP8266_Clear();
    ESP8266_SEND((unsigned char *)cmd, strlen(cmd));
    HAL_Delay(5000);

    uint16_t last_checked_index = 0;
    uint16_t received_len = 0;
    uint16_t res_len = strlen(res);  // 提前计算关键词长度

    while(time--) {
        if(ESP8266_WaitRecive(&received_len) == REV_OK) {
            last_checked_index = received_len;  // 记录物理长度
            
            // 二进制安全检索（避免使用strlen/strstr）
            for(uint16_t i = 0; i <= received_len - res_len; i++) {
                if(memcmp(&ESP8266_BUF[i], res, res_len) == 0) {
                    return 0;  // 匹配成功
                }
            }
        }
        HAL_Delay(1);
    }
    return 1;  // 超时
}


//==========================================================
//	函数名称：	ESP8266_Init
//	函数功能：	初始化ESP8266
//	入口参数：	无
//	返回参数：	无
//	说明：		
//==========================================================

void ESP8266_OneNet_Init(void)
{	
	char cmd[512]; // 确保足够容纳长token
	
	ESP8266_Clear();

	while(ESP8266_SendCmd("AT+RST\r\n", "ready", 5000))       //重启模块
	HAL_Delay(1000);
	
	while(ESP8266_SendCmd("AT\r\n", "OK", 200))         //测试
	HAL_Delay(1000);
	
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))       
	HAL_Delay(1000);

	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))       
	HAL_Delay(1000);
	
	while(ESP8266_SendCmd("AT+CWJAP=\"LK\",\"1234567890\"\r\n", "GOT IP", 200))//设置显示名称:LK,密码:1234567890
	HAL_Delay(1000);

		
	memset(cmd, 0, 512);
	
	snprintf(cmd, sizeof(cmd), MQTT_CONFIG_CMD, DEVICENAME, PRODUCEKEY, TOKEN);  //三要素发送
	while(ESP8266_SendCmd(cmd, "OK", 2000)) 
	HAL_Delay(1000);
	
	
	while(ESP8266_SendCmd("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n", "OK", 2000))
	HAL_Delay(1000);	
	
	memset(cmd, 0, 512);

	snprintf(cmd, sizeof(cmd), MQTT_SUBSCRIBE_CMD, PRODUCEKEY, DEVICENAME, "post/reply", 2);// 订阅post/reply主题(QoS 0)
	while(ESP8266_SendCmd(cmd, "OK", 1000))
  HAL_Delay(1000);
	
	memset(cmd, 0, 512);

	snprintf(cmd, sizeof(cmd), MQTT_SUBSCRIBE_CMD,PRODUCEKEY, DEVICENAME, "set", 2);// 订阅set主题(QoS 2)//2可以发送小数
	while(ESP8266_SendCmd(cmd, "OK", 1000))
  HAL_Delay(1000);
	

}
//==========================================================
//	函数名称：	ReceiveData_OneNet
//	函数功能：	处理WiFi端接收到的数据
//	入口参数：	无
//	返回参数：	接收到的数据值
//	说明：name：发送端键值
//==========================================================
u16 ReceiveData_OneNet(unsigned char *dataPtr,const char *name)
{
	
	char *str1 = 0, i;
	int setValue = 0;
	char setvalue[3] = {0};
	if(strstr((char *)dataPtr,name)!=NULL)
	{
		str1 = strstr((char *)dataPtr,name);
		while (*str1 != ':')
		{
			str1 = str1 + 1;
			HAL_Delay(10);
		}
		str1 = str1 + 1;
		i = 0;
		while (*str1 >= '0' && *str1 <= '9')
		{
			setvalue[i] = *str1;
			i++;
			str1++;
			if (*str1 == '\r')
				break;
			HAL_Delay(10);
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

/**
 * @brief 发送浮点型数据到MQTT主题
 * @param value 要发送的浮点数值
 * @param property_name 数据属性名（如"temp"、"humi"等）
 * @return 0表示成功
 */

int ESP8266_MQTTSend(float temp)
{
    char SendArray[512]; 
    
//    snprintf(SendArray, sizeof(SendArray),
//        "AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\","
//        "\""  // JSON开始引号
//        "{\\\"id\\\": \\\"123\\\"\\,\\\"params\\\":{"
//            "\\\"button\\\":{\\\"value\\\":%.1f}\\,"
//            "\\\"temp\\\":{\\\"value\\\":%d}\\,"
//            "\\\"humi\\\":{\\\"value\\\":%d}\\,"
//            "\\\"light\\\":{\\\"value\\\":%d}\\,"
//            "\\\"soil\\\":{\\\"value\\\":%d}\\,"
//            "\\\"CO2\\\":{\\\"value\\\":%d}"
//        "}}"
//        "\""  // JSON结束引号
//        ",0,0\r\n",
//        PRODUCEKEY, DEVICENAME,
//        button_value, temp, humi, light, soil, CO2
//    );

					snprintf(SendArray, sizeof(SendArray),
							"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\","
							"\""  // JSON开始引号
							"{\\\"id\\\": \\\"123\\\"\\,\\\"params\\\":"
							"{\\\"temp\\\":{\\\"value\\\":%.1f}}}"
							"\""  // JSON结束引号
							",0,0\r\n",
							PRODUCEKEY, DEVICENAME, temp
					);
	
    // 发送合并后的指令
    ESP8266_SEND(SendArray, strlen(SendArray));
    
    // 清空缓冲区
    delay_ms(500);
    ESP8266_Clear();
    
    return 0;

}

int ESP8266_MQTTSendReturn(bool temp)
{
    char SendArray[256]; 
					snprintf(SendArray, sizeof(SendArray),
							"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\","
							"\""  // JSON开始引号
							"{\\\"id\\\": \\\"123\\\"\\,\\\"params\\\":"
							"{\\\"button\\\":{\\\"value\\\":%s}}}"
							"\""  // JSON结束引号
							",0,0\r\n",
							PRODUCEKEY, DEVICENAME, temp
					);
	
    // 发送合并后的指令
    ESP8266_SEND(SendArray, strlen(SendArray));
    
    // 清空缓冲区
    delay_ms(500);
    ESP8266_Clear();
    
    return 0;

}


int contains_substring(const u8 *main_string, const char *substring)
{
	if (strstr((const char *)main_string, substring) != NULL)
	{
		return 1;
	}
	else
	{
		return 0; 
	}
}

int ESP8266_Get(const char *res1, const char *res2)
{

	if (1 == contains_substring(ESP8266_BUF, res1))
	{
		if (1 == contains_substring(ESP8266_BUF, res2))
		{
			return 1; 
		}
	}
	return 0;
}

//获取状态
uint16_t button_st,button_at;
float temp_set;
int ESP8266_Get_state(void)
{

	if (1 == ESP8266_Get("button", "false"))
	{
		button_st++;
	}
	else if (1 == ESP8266_Get("button", "true"))
	{
		button_at++;
	}
	return 1;
}
//获取数值
int ESP8266_Get_data(const char *substring)
{
	  char *start_ptr, *end_ptr;
    char value_str[16]; // 存储提取的数值字符串
    float value = 0.0f;

    // 1. 构造搜索模式（如"\"temp\":"）
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":", substring);

    // 2. 查找字段位置
    start_ptr = strstr((char *)ESP8266_BUF, pattern);
    if (start_ptr == NULL) return 0.0f;

    // 3. 定位到数值开始位置（跳过字段名和引号）
    start_ptr += strlen(pattern);

    // 4. 提取数值部分（可能以逗号、}、空格或换行结尾）
    end_ptr = strpbrk(start_ptr, ",} \n"); // 查找逗号、}、空格或换行符
    if (end_ptr == NULL) return 0.0f;

    // 5. 拷贝数值字符串并转换为float
    strncpy(value_str, start_ptr, end_ptr - start_ptr);
    value_str[end_ptr - start_ptr] = '\0';
    value = atof(value_str);

    return value;
}

void ESP8266_Get_information(void)
{
	ESP8266_Get_state();
	if(ESP8266_Get_data("temp_SET"))
	{
		temp_set = ESP8266_Get_data("temp_SET");
	}	
	ESP8266_Clear();
	
}










