/**
 ****************************************************************************************************
 * @file        main.c
 * @author      
 * @version     V1.0
 * @date        
 * @brief       公版程序组
 ****************************************************************************************************
 ****************************************************************************************************
 */
#include "main.h"
extern uint8_t data_index;
int main(void)
{

    HAL_Init();                                 /* 初始化HAL库 */
    SystemClock_Config(RCC_PLL_MUL9);        	  /* 设置时钟, 72Mhz */
    delay_init(72);                             /* 延时初始化 */
		MX_IIC_Init();
    Uart_Init();   

	/* 串口初始化为115200 */
    MX_ADC_Init();															/* ADC初始化 */
    MX_SPIx_Init();															/* SPI初始化 */
    MX_TIMx_Init();															/* 定时器初始化 */
		MX_Hard_Init();                             /* 所有外设初始化 */ 

#if(WDT_RUN)	  	
  IWDG_init(IWDG_PRESCALER_64, 625*6);//4S      /* 看门狗初始化 */
	IWDG_feed();                          				/* 喂狗 */  
#endif	
		char display_str[16];
    while (1)
    {
			UsartSendReceiveData();
			UsartReceive();
			Keypad_Handle();  //按键
 		  addnum();
			Keypad_Handle();  //按键
			redycenum();
			Keypad_Handle();  //按键
			Display();	
			Keypad_Handle();  //按键
			DISplaysetvalue();
//			Keypad_Handle();  //按键
//			Displaysetvalue();
			Keypad_Handle();  //按键
			CotorFun();
#if(WDT_RUN)					
				IWDG_feed();														/* 喂狗 */ 
#endif				
    }
		
}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
