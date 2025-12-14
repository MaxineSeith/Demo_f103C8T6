#include "main.h"
#include "oled.h"


#define OLED_ADDRESS 0x78 // OLED的I2C地址，假设为0x78，根据实际硬件调整

/***************************************************************************************
 * 函数名        : OLED_Init
 * 描述          : 初始化OLED显示屏
 * 输入          : 无
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_Init(void)
{
	I2C_Configuration(); // 使用提供的I2C初始化函数
	delay_ms(100); // 延时100ms，确保硬件初始化完成
	
	// 发送OLED初始化命令
	WriteCmd(0xAE); // Display OFF
	WriteCmd(0x20); // Set Memory Addressing Mode
	WriteCmd(0x10); // Horizontal Addressing Mode
	WriteCmd(0xb0); // Set Page Start Address for Page Addressing Mode
	WriteCmd(0xc8); // Set COM Output Scan Direction
	WriteCmd(0x00); // Set low column address
	WriteCmd(0x10); // Set high column address
	WriteCmd(0x40); // Set start line address
	WriteCmd(0x81); // Set contrast control register
	WriteCmd(0xff); // Contrast value
	WriteCmd(0xa1); // Set segment re-map
	WriteCmd(0xa6); // Set normal display
	WriteCmd(0xa8); // Set multiplex ratio
	WriteCmd(0x3F); // 1/64 duty
	WriteCmd(0xa4); // Output follows RAM content
	WriteCmd(0xd3); // Set display offset
	WriteCmd(0x00); // No offset
	WriteCmd(0xd5); // Set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); // Set divide ratio
	WriteCmd(0xd9); // Set pre-charge period
	WriteCmd(0x22); // Pre-charge period value
	WriteCmd(0xda); // Set COM pins hardware configuration
	WriteCmd(0x12); // COM pins configuration
	WriteCmd(0xdb); // Set VCOMH
	WriteCmd(0x20); // VCOMH level
	WriteCmd(0x8d); // Set DC-DC enable
	WriteCmd(0x14); // Enable DC-DC
	WriteCmd(0xaf); // Turn on OLED panel
}

/***************************************************************************************
 * 函数名        : OLED_SetPos
 * 描述          : 设置OLED显示的坐标
 * 输入          : x: x坐标, y: y坐标
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_SetPos(unsigned char x, unsigned char y)
{
	WriteCmd(0xb0 + y); // Set page address
	WriteCmd(((x & 0xf0) >> 4) | 0x10); // Set high column address
	WriteCmd((x & 0x0f) | 0x01); // Set low column address
}

/***************************************************************************************
 * 函数名        : OLED_Fill
 * 描述          : 填充整个OLED屏幕
 * 输入          : fill_Data: 填充数据
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_Fill(unsigned char fill_Data)
{
	unsigned char m, n;
	for (m = 0; m < 8; m++) {
		WriteCmd(0xb0 + m); // Set page address
		WriteCmd(0x00); // Set low column address
		WriteCmd(0x10); // Set high column address
		for (n = 0; n < 128; n++) {
			WriteDat(fill_Data); // Fill the screen
		}
	}
}

/***************************************************************************************
 * 函数名        : OLED_CLS
 * 描述          : 清空OLED屏幕
 * 输入          : 无
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_CLS(void)
{
	OLED_Fill(0x00); // Clear the screen by filling with 0x00
}

/***************************************************************************************
 * 函数名        : OLED_ON
 * 描述          : 唤醒OLED显示屏
 * 输入          : 无
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_ON(void)
{
	WriteCmd(0x8D); // Set charge pump
	WriteCmd(0x14); // Enable charge pump
	WriteCmd(0xAF); // Turn on OLED display
}

/***************************************************************************************
 * 函数名        : OLED_OFF
 * 描述          : 关闭OLED显示屏（休眠模式）
 * 输入          : 无
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_OFF(void)
{
	WriteCmd(0x8D); // Set charge pump
	WriteCmd(0x10); // Disable charge pump
	WriteCmd(0xAE); // Turn off OLED display
}

/***************************************************************************************
 * 函数名        : OLED_ShowStr
 * 描述          : 显示字符串
 * 输入          : x, y: 显示位置, ch[]: 字符串, TextSize: 字符大小 (1:6x8, 2:8x16)
 * 输出          : 无
 * 返回          : 无
 ***************************************************************************************/
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0, i = 0, j = 0;
	switch (TextSize) {
	case 1:
		while (ch[j] != '\0') {
			c = ch[j] - 32;
			if (x > 126) { x = 0; y++; }
			OLED_SetPos(x, y);
			for (i = 0; i < 6; i++) {
				WriteDat(F6x8[c][i]); // Write character
			}
			x += 6;
			j++;
		}
		break;
	case 2:
		while (ch[j] != '\0') {
			c = ch[j] - 32;
			if (x > 120) { x = 0; y++; }
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++) {
				WriteDat(F8X16[c * 16 + i]); // Write character part 1
			}
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++) {
				WriteDat(F8X16[c * 16 + i + 8]); // Write character part 2
			}
			x += 8;
			j++;
		}
		break;
	}
}
