#pragma once

#include <esp_adc/adc_oneshot.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void Adc_PortInit();
    float Adc_GetBatteryVoltage();
    uint8_t Adc_GetBatteryLevel();
#ifdef __cplusplus
}
#endif