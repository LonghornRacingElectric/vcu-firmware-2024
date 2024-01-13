#ifndef VCU_FIRMWARE_2024_CELLULAR_H
#define VCU_FIRMWARE_2024_CELLULAR_H

#include <string>
#include "VcuParameters.h"
#include "main.h"
#include "VcuModel.h"
#include "hvc.h"
#include "pdu.h"
#include "inverter.h"
#include "adc.h"
#include "wheelspeeds.h"
#include "all_imus.h"
#include "gps.h"


extern UART_HandleTypeDef huart7;


// private helper methods
static void cellular_sendAT(const std::string& command);

static void cellular_receiveAT();

static bool cellular_areParametersUpdated();

static void cellular_updateParameters(VcuParameters *vcuCoreParameters);

static void cellular_sendTelemetry();


// public methods
void cellular_init();

void cellular_periodic(VcuParameters *vcuCoreParameters,
                       VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                       ImuData *imuData, GpsData *gpsData);

#endif //VCU_FIRMWARE_2024_CELLULAR_H
