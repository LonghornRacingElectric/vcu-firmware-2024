#ifndef HVC_FIRMWARE_2024_USB_H
#define HVC_FIRMWARE_2024_USB_H

#include "usart.h"
#include "adc.h"
#include "VcuModel.h"
#include <string>

void println(float f);
void println(std::string& s);
void serialControlInit();
void serialControlPeriodic();
void printLiveControls(const AnalogVoltages& analogVoltages, const VcuOutput& vcuOutput);

extern "C" void serialControlHandleUartError(UART_HandleTypeDef* huart);

#endif //HVC_FIRMWARE_2024_USB_H
