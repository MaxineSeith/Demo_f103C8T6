#include "main.h"
#include "ina219.h"

IIC_HardWareTypedef iicina219;

uint16_t ina219_calibrationValue;
uint16_t ina219_currentDivider_mA;
float ina219_powerMultiplier_mW;

float ina219_currentDivider_uA;
float ina219_powerMultiplier_mW;

/**
 * @brief  The IIC reads 16bit data from the specified register address.
 * @param  ina219 Slave configuration structure of the IIC.
 * @param  registerAddress Internal memory address.
 * @return 16 bit register data.
 */
uint16_t INA219_ReadDataForRegister_16Bits(uint8_t Bddr, uint8_t registerAddress)
{
  uint8_t Value[2];
	IIC_Read_Device_2Byte_Len(Bddr,registerAddress,2,Value,iicina219);
  return ((Value[0] << 8) | Value[1]);
}

/**
 * @brief  Writes 16 bits of data to the register.
 * @param  ina219 Slave configuration structure of the IIC.
 * @param  registerAddress Internal memory address.
 * @param  Value 16 bits of data to be written.
 */



void INA219_WriteDataToRegister_16Bits(uint8_t Bddr, uint8_t registerAddress, uint16_t Value)
{
	IIC_Write_Device_2Byte(Bddr,registerAddress,2,Value,iicina219);
}

/**
 * @brief  Read watt.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Read watt, unit mW.
 */
uint16_t INA219_Readwatt(uint8_t Bddr)
{
	uint16_t result = INA219_ReadDataForRegister_16Bits(Bddr, INA219_REG_POWER);
	return result = (result*ina219_powerMultiplier_mW);                 
}


/**
 * @brief  Read bus voltage.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Read voltage value, unit mV.
 */
uint16_t INA219_ReadBusVoltage(uint8_t Bddr)
{
  uint16_t result = INA219_ReadDataForRegister_16Bits(Bddr, INA219_REG_BUS_VOLTAGE);
  /* return mV */
  return ((result >> 3  ) * 4);
}

/**
 * @brief  Read current register value.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Current register value.
 */
uint16_t INA219_ReadCurrentRaw(uint8_t Bddr)
{
  uint16_t result = INA219_ReadDataForRegister_16Bits(Bddr, INA219_REG_CURRENT);

  return (result);
}

/**
 * @brief  Read current register value, unit mA.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Current value.
 */
uint16_t INA219_ReadCurrent_mA(uint8_t Bddr)
{
	  uint16_t result = 0,result1 = 0;  
	  HAL_Delay(10);	
    result = INA219_ReadCurrentRaw(Bddr);
	  if(result>=0x3332)result = 0;
    return   result1 = (result / ina219_currentDivider_mA);
}


uint16_t INA219_ReadCurrent_uA(uint8_t Bddr)
{
	  uint16_t result = 0,result1 = 0;  
	  HAL_Delay(10);	
    result = INA219_ReadCurrentRaw(Bddr);
	  if(result>=0x3332)result = 0;
    return   result1 = (result / ina219_currentDivider_uA);
}

/**
 * @brief  Read current register value, unit mV.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Shunt Voltage value.
 */
float	INA219_ReadShuntVoltage_mV(uint8_t Bddr)
{
  uint16_t result = 0;
  float result_f = 0;
  /* When multiple sign bits are present, they will all be the same value.
   * Negative numbers are represented in 2's complement format.
   * Generate the 2's complement of a negative number by complementing the absolute value binary number and adding 1.
   * Extend the sign, denoting a negative number by setting the MSB = 1.
   * Extend the sign to any additional sign bits to form the 16-bit word. */
	 result = INA219_ReadDataForRegister_16Bits(Bddr, INA219_REG_SHUNT_VOLTAGE);
	
  if(result > MAX_SHUNT_RANGE)
  {
    result = 65536 - MAX_SHUNT_RANGE;
  }

  /* Shunt voltage, unit mV. */
  return result_f = (float)result / 100;
__NOP();
}

/**
 * @brief  INA219 system reset.
 * @param  ina219 Slave configuration structure of the IIC.
 */
void INA219_Reset(uint8_t Bddr)
{
	INA219_WriteDataToRegister_16Bits(Bddr, INA219_REG_CONFIG, INA219_CONFIG_RESET);
  HAL_Delay(1);
}

/**
 * @brief Set calibration register.
 * @param ina219 Slave configuration structure of the IIC.
 * @param calibrationData Calibrated data.
 */
void INA219_SetCalibration(uint8_t Bddr, uint16_t calibrationData)
{
		uint8_t regAddr[2]={0,0};
    INA219_WriteDataToRegister_16Bits(Bddr, INA219_REG_CALIBRATION, calibrationData);
}

/**
 * @brief  Gets the value of the configuration register.
 * @param  ina219 Slave configuration structure of the IIC.
 * @return Configuration Register value.
 */
uint16_t INA219_GetConfigInfo(uint8_t Bddr)
{
  uint16_t result = INA219_ReadDataForRegister_16Bits(Bddr, INA219_REG_CONFIG);
  return result;
}

/**
 * @brief Set configuration register.
 * @param ina219 Slave configuration structure of the IIC.
 * @param configData Configuration data.
 */
void INA219_SetConfig(uint8_t Bddr, uint16_t configData)
{
  INA219_WriteDataToRegister_16Bits(Bddr, INA219_REG_CONFIG, configData);
}

/**
 * @brief The measurement results are calibrated. Voltage range is 16V, Current range is 8A.
 * @param ina219 Slave configuration structure of the IIC.
 */
void INA219_SetCalibration_16V_8A(uint8_t Bddr)
{
  uint16_t configInfo = INA219_CONFIG_VOLTAGE_RANGE_16V |
												INA219_CONFIG_GAIN_1_40MV | 
												INA219_CONFIG_BADCRES_12BIT |
                        INA219_CONFIG_SADCRES_12BIT_1S_532US |
                        INA219_CONFIG_MODE_SVOLT_CONTINUOUS;

	
	    // 计算 Calibration 值（0.1Ω 采样电阻，8A 最大电流）
    ina219_calibrationValue = 0x0690; // 1680 (十进制)

    // 计算电流和功率的转换系数
    ina219_currentDivider_mA = 4.0f;  // 1mA = 4 * Current_LSB (因为 244μA/LSB ≈ 4LSB/mA)
    ina219_powerMultiplier_mW = 0.2f; // 1mW = 0.2 * Power_LSB (根据公式推导)

  	INA219_SetConfig(Bddr, configInfo);	
		HAL_Delay(10);
		INA219_SetCalibration(Bddr, ina219_calibrationValue);
  
}
void INA219_SetCalibration_32V_8A(uint8_t Bddr)
{
  uint16_t configInfo = INA219_CONFIG_VOLTAGE_RANGE_32V |
												INA219_CONFIG_GAIN_8_320MV | 
												INA219_CONFIG_BADCRES_12BIT |
                        INA219_CONFIG_SADCRES_12BIT_1S_532US |
                        INA219_CONFIG_MODE_SVOLT_CONTINUOUS;

  // Current_LSB = Maximum Expected Current / 2^15 = (320 / 50) / 2^15 = 0.0002  200uA
  // Cal = 0.04096 / (Current_LSB / R) = 0.04096 / (0.000097A * 0.05R) = 8445 = 0x20FD
  // Calibration Register = 8445
     ina219_calibrationValue = 4096;	

  // 1mA = Current_LSB * bits = 97uA * 10bit (5 bit/mA)
		ina219_currentDivider_mA = 5;	

		// 1mW = Power_LSB * bits = 4mW * 0.25bit (0.25f bit/mW)
		ina219_powerMultiplier_mW = 0.25f;
	
  	INA219_SetConfig(Bddr, configInfo);	
		HAL_Delay(10);
		INA219_SetCalibration(Bddr, ina219_calibrationValue);
  
}

void INA219_SetCalibration_16V_800mA(uint8_t Bddr)
{
  uint16_t configInfo = INA219_CONFIG_VOLTAGE_RANGE_16V |
												INA219_CONFIG_GAIN_1_40MV | 
												INA219_CONFIG_BADCRES_12BIT |
                        INA219_CONFIG_SADCRES_12BIT_1S_532US|
                        INA219_CONFIG_MODE_SVOLT_CONTINUOUS;

//  // Current_LSB = Maximum Expected Current / 2^15 = (40 / 50) / 2^15 = 0.0000244A=0.0244mA
//  // Cal = 0.04096 / (Current_LSB / R) = 0.04096 / (0.0002A * 0.05R) = 33554 = 0x8312
//  // Calibration Register = 20480
//  ina219_calibrationValue = 20480;
     ina219_calibrationValue = 33554;	

  // 1mA = Current_LSB * bits = 24.4uA * 10bit (5 bit/mA)
		ina219_currentDivider_mA = 41;	

		// 1mW = Power_LSB * bits = 4mW * 0.25bit (0.25f bit/mW)
		ina219_powerMultiplier_mW = 0.25f;
	
  	INA219_SetConfig(Bddr, configInfo);	
		HAL_Delay(10);
		INA219_SetCalibration(Bddr, ina219_calibrationValue);
  
}

/**
 * @brief  Ina219 driver initialization
 * @param  ina219 Slave configuration structure of the IIC.
 * @param  i2c Pointer to a I2C_HandleTypeDef structure that contains
 *             the configuration information for the specified I2C.
 * @param  Address  Configuration data.
 * @return status.
 */
uint16_t resulto[4] = {0,0,0,0};

uint8_t INA219_Init(uint8_t Bddr)
{

		INA219_SetCalibration_16V_8A(Bddr);	
	  return 0;
}




void INA219a_Init(void)
{
	INA219_Init(INA219_ADDRESS);

}

void Get_INA219Data(void)
{
	uint16_t vbus,vshunt,current,watt;
	 vbus = INA219_ReadBusVoltage(INA219_ADDRESS);        //获取供电电压
   vshunt  = INA219_ReadShuntVoltage_mV(INA219_ADDRESS); //获取电压
   current = INA219_ReadCurrent_mA(INA219_ADDRESS);     //获取电流
	 watt=	INA219_Readwatt(INA219_ADDRESS);
}	
