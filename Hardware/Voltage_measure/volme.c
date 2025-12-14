#include "main.h"
#include "volme.h"

Array vol={{0},0};
// 校准参数（根据实测数据计算）


float Get_voltage(void)
{
    uint16_t raw_adc = Get_Adc2()*0.004029304029304f;

    if (raw_adc <= 0) return 0;
    else if (raw_adc > 4.2f) raw_adc = 4.2f;
    
    return Wavefiltering1(raw_adc, &vol, 50);
}

