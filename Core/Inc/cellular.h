#ifndef VCU_FIRMWARE_2024_CELLULAR_H
#define VCU_FIRMWARE_2024_CELLULAR_H

#include <string>
#include "VcuParameters.h"
#include "stm32h7xx_hal.h"


extern UART_HandleTypeDef huart7;


typedef struct TelemetryPacket {
    float someVoltage;
    float someTorque;
    uint32_t someErrors;
} TelemetryPacket;


// private helper methods
static void cellular_sendAT(std::string command);
// ... more will probably be needed


// public methods
void cellular_sendTelemetry(TelemetryPacket* telemetry);
bool cellular_areParametersUpdated();
void cellular_updateParameters(VcuParameters* vcuParameters);
void cellular_periodic();

#endif //VCU_FIRMWARE_2024_CELLULAR_H
