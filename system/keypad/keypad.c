#include "main.h"
#include "keypad.h"



#if(button_Set)


/**
 * @brief       按键初始化函数
 * @param       无
 * @retval      无
 */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    KEY0_GPIO_CLK_ENABLE();                                     /* KEY0时钟使能 */
    KEY1_GPIO_CLK_ENABLE();                                     /* KEY1时钟使能 */
    KEY2_GPIO_CLK_ENABLE();                                     /* KEY1时钟使能 */
    KEY3_GPIO_CLK_ENABLE();                                     /* KEY1时钟使能 */
	
	
    GPIO_InitStruct.Pin = KEY0_GPIO_PIN;                       /* KEY0引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                    /* 输入 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                        /* 上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    HAL_GPIO_Init(KEY0_GPIO_PORT, &GPIO_InitStruct);           /* KEY0引脚模式设置,上拉输入 */

    GPIO_InitStruct.Pin = KEY1_GPIO_PIN;                       /* KEY1引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                    /* 输入 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                        /* 上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);           /* KEY1引脚模式设置,上拉输入 */

    GPIO_InitStruct.Pin = KEY2_GPIO_PIN;                       /* KEY1引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                    /* 输入 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                        /* 上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    HAL_GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStruct);           /* KEY1引脚模式设置,上拉输入 */	

    GPIO_InitStruct.Pin = KEY3_GPIO_PIN;                       /* KEY1引脚 */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                    /* 输入 */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                        /* 上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    HAL_GPIO_Init(KEY3_GPIO_PORT, &GPIO_InitStruct);           /* KEY1引脚模式设置,上拉输入 */	
		

}

/**
 * @brief       按键扫描函数
 * @note        该函数有响应优先级(同时按下多个按键): WK_UP > KEY1 > KEY0!!
 * @param       mode:0 / 1, 具体含义如下:
 *   @arg       0,  不支持连续按(当按键按下不放时, 只有第一次调用会返回键值,
 *                  必须松开以后, 再次按下才会返回其他键值)
 *   @arg       1,  支持连续按(当按键按下不放时, 每次调用该函数都会返回键值)
 * @retval      键值, 定义如下:
 *              KEY0_PRES, 1, KEY0按下
 *              KEY1_PRES, 2, KEY1按下
 *              WKUP_PRES, 3, WKUP按下
 */
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  /* 按键按松开标志 */
    uint8_t keyval = 0;

    if (mode) key_up = 1;       /* 支持连按 */

    if (key_up && (KEY0 == 0 || KEY1 == 0 ||KEY2 == 0 ||KEY3 == 0 ))  /* 按键松开标志为1, 且有任意一个按键按下了 */
    {
        delay_ms(10);           /* 去抖动 */
        key_up = 0;

        if (KEY0 == 0)  keyval = KEY0_PRES;
        if (KEY1 == 0)  keyval = KEY1_PRES;
				if (KEY2 == 0)  keyval = KEY2_PRES;
				if (KEY3 == 0)  keyval = KEY3_PRES;
			
    }
    else if (KEY0 == 1 && KEY1 == 1&& KEY2 == 1&& KEY3 == 1 ) /* 没有任何按键按下, 标记按键松开 */
    {
        key_up = 1;
    }

    return keyval;              /* 返回键值 */
}
extern u8 mode;

float voltage,power;
float volMax = 2.5;
uint16_t current;
uint16_t currentMax = 180;
uint8_t key_num[4];
uint8_t Key_num[4];
void Process_Button_Events(void)
{
				uint8_t key;
	
        key = key_scan(0);                  /* 得到键值 */

        if (key)
        {
            switch (key)
            {
							 case KEY0_PRES:   
												key_num[0]++;
							 					OLED_CLS();
												if(key_num[0]>=255)key_num[0]=0;	
                    break;
							 
							  case KEY1_PRES:  
										Key_num[1]++;								
										key_num[1]++;
						
                    break;
								
                case KEY2_PRES:            
											key_num[2]++;
											Key_num[2]++;											
                    break;

                case KEY3_PRES:             
											key_num[3]++;
							        if(key_num[3]>=255)key_num[3]=0;
                    break;


            } 
        }
        else
        {
            delay_ms(10);
        }
    
}

#endif




void Keypad_Handle(void)
{
#if(key_pad_Set)	
	Keypad_HandleKeyPress(Keypad_GetKey());	
#endif	
#if(button_Set)	
	  Process_Button_Events();
#endif	
}	



#if(0)	
//废案，测试正常，可以使用，暂无扩展功能舍弃
/**
 * @brief 检测按键是否按下（低电平有效，带消抖）
 * @return 1: 按下, 0: 未按下
 */
uint8_t Key_Scan(void) 
{
    static uint8_t key_up = 1;  // 按键松开标志

    if (key_up && (HAL_GPIO_ReadPin(Button1_GPIO_PORT, Button1_GPIO_PIN) == GPIO_PIN_RESET)) 
			{
        HAL_Delay(10);  // 消抖延时（10ms）
        key_up = 0;      // 标记按键已按下
        if (HAL_GPIO_ReadPin(Button1_GPIO_PORT, Button1_GPIO_PIN) == GPIO_PIN_RESET) 
				{
            return 1;    // 确认按下
        }
      } 
			else if (HAL_GPIO_ReadPin(Button1_GPIO_PORT, Button1_GPIO_PIN) == GPIO_PIN_SET)
				{
        key_up = 1;      // 按键已松开
        }
    return 0;
}




// 获取当前按键状态（带消抖）
uint8_t Button_GetState(void)
{
	uint8_t state = HAL_GPIO_ReadPin(Button1_GPIO_PORT, Button1_GPIO_PIN);
	
	if(state != last_state) 
	{
		debounce_counter++;
		if(debounce_counter >= 5) {  // 消抖时间约5ms
			debounce_counter = 0;
			last_state = state;
			current_state = state;
		}
	}else{
		debounce_counter = 0;
	}
	
	return current_state;
}

// 获取按键事件（按下、释放、短按、长按）
uint8_t Button_GetEvent(void)
{
	static uint8_t last_btn_state = BUTTON_RELEASED;
	uint8_t btn_state = Button_GetState();
	uint8_t event = BTN_EVENT_NONE;
	
	// 状态变化检测
	if (btn_state != last_btn_state) {
		if (btn_state == BUTTON_PRESSED) {
			// 按键按下
			event = BTN_EVENT_PRESSED;
			press_start_time = HAL_GetTick();
			long_press_detected = 0;
		} else {
			// 按键释放
			event = BTN_EVENT_RELEASED;
			if (!long_press_detected && (HAL_GetTick() - press_start_time < 1000)) {
				event = BTN_EVENT_SHORT_PRESS;
			}
		}
		last_btn_state = btn_state;
	} 
	// 长按检测
	else if (btn_state == BUTTON_PRESSED && !long_press_detected) {
		if (HAL_GetTick() - press_start_time >= 1000) {  // 长按1秒
			event = BTN_EVENT_LONG_PRESS;
			long_press_detected = 1;
		}
	}
	
	return event;
}

uint16_t key_x;
/**
 * @brief 处理按键事件
 * @param btn_event 按键事件类型
 */
void Handle_ButtonEvent(uint8_t btn_event)
{
    switch (btn_event) {
        case BTN_EVENT_PRESSED:  // 按键按下事件处理
						key_x=1;
            break;
        
        case BTN_EVENT_RELEASED:	// 按键释放事件处理
            key_x=2;
            break;
        
        case BTN_EVENT_SHORT_PRESS: // 短按事件处理
             key_x=3;
            
            break;
        
        case BTN_EVENT_LONG_PRESS:	// 长按事件处理
             key_x=4;
            // 可以在这里添加进入配置模式等操作
            break;
        
        default:
            break;
    }
}
#endif	

u8 mode = 0,warningFlag = 0,load = 0;
char display[16];//显示存储数组
uint8_t data_index = 0;
char str[50];
void addnum(void)
{
	u8 i;
		if(Key_num[1]>=  1 && volMax < 25 && (key_num[0] % 3 == 1))
		{
			volMax+=0.5;
			Key_num[1] = 0;
		}
		if(Key_num[1] >=  1 && currentMax< 3200 && (key_num[0] % 3 == 2))
		{
			currentMax+=10;
			Key_num[1] = 0;
		}
		

}
void redycenum(void)
{
	u8 i;
	if((key_num[0] % 3 == 0) && (voltage > volMax && current < currentMax) && (Key_num[2] >= 1))
	{
		load = !load;
		if(load == 0)RELAY(OFF);
		else RELAY(ON);
		Key_num[2] = 0;
	}	
	if(volMax > 0.5 && (key_num[0] % 3 == 1)&& Key_num[2] >=  1)
	{
		volMax-=0.5;	
		Key_num[2] = 0;
	}			
	if(currentMax> 10 && (key_num[0] % 3 == 2)&& Key_num[2] >=  1)
	{
		currentMax-=10;
		Key_num[2] = 0;		
	}
}


void Display(void)
{
		u8 i;
	if(key_num[0] % 3 == 0)
	{
		voltage = Get_voltage(); //获取电压
		current = INA219_ReadCurrent_mA(INA219_ADDRESS);     //获取电流
		

			for(i=0;i<2;i++)OLED_ShowCN(i*16+0,0,i+0,0);//显示中文：电压
			for(i=0;i<2;i++)OLED_ShowCN(i*16+0,2,i+2,0);//显示中文：电流
			for(i=0;i<2;i++)OLED_ShowCN(i*16+0,4,i+8,0);//显示中文：功率
		
			power = (voltage * current) / 1000;//计算功率
		
			sprintf(display,":%2.1fV ",voltage);
			OLED_ShowStr(32, 0,(u8 *)display, 2,0);
			sprintf(display,":%4dmA",current);
			OLED_ShowStr(32, 2,(uint8_t *)display, 2,0);
			sprintf(display,":%2.1fW ",power);
			OLED_ShowStr(32, 4,(u8 *)display, 2,0);
	}		
}

void DISplaysetvalue(void)
{
	u8 i;
	if(key_num[0] % 3 == 1) 
	{
		for(i=0;i<2;i++)OLED_ShowCN(i*16+32,0,i+4,0);//显示中文：设置
		for(i=0;i<2;i++)OLED_ShowCN(i*16+64,0,i+0,0);//显示中文：电压
		sprintf(display,"%2.1fV",volMax);
		OLED_ShowStr(44,3,(uint8_t *)display,2,0);
	}		
	if(key_num[0] % 3 == 2) 
	{
		for(i=0;i<2;i++)OLED_ShowCN(i*16+32,0,i+4,0);//显示中文：设置
		for(i=0;i<2;i++)OLED_ShowCN(i*16+64,0,i+2,0);//显示中文：电流
		sprintf(display,"%4dmA",currentMax);
		OLED_ShowStr(44,3,(uint8_t *)display,2,0);
	}		
}
void Displaysetvalue(void)
{

}
void CotorFun(void)
{	

		if(key_num[0] % 3 == 0)
		{
			if(voltage < volMax)
				{
					BEEP_DOING();
					RELAY(OFF);
					OLED_ShowStr(40,0,(uint8_t *)"      ",2,0);
					warningFlag |= (1 << 0);// 电压报警标志位置1
				}else{
					warningFlag &= ~(1 << 0);// 电压报警标志位置0
				}
				if(current > currentMax)
				{
					BEEP_DOING();
					RELAY(OFF);
					OLED_ShowStr(40,2,(uint8_t *)"      ",2,0);	
					warningFlag |= (1 << 1);// 电流报警标志位置1
				}else{
					warningFlag &= ~(1 << 1);// 电流报警标志位置0
				}
		}
}
