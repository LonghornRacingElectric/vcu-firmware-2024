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


// strings
static std::string CELL_OK = "\r\nOK\r\n";


// private helper methods
static void cellular_send(std::string* command);

static bool cellular_receive(std::string& expected, bool care);

static void cellular_receiveAny(int size, std::string& response, int time);

static void cellular_sendAndExpectOk(std::string* command);

static bool cellular_areParametersUpdated();

static void cellular_updateParameters(VcuParameters *vcuCoreParameters);

static void cellular_sendTelemetry();

static void cellular_disableEcho();

static void cellular_findTMobileHSNCode(std::string& code);

/**
 * only needs to be done once, then it's stored in the module's NVM
 */
static void cellular_setBaudRate();

static void cellular_testConnection();

static void cellular_mqttInit();


// public methods
void cellular_init();

void cellular_periodic(VcuParameters *vcuCoreParameters,
                       VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                       ImuData *imuData, GpsData *gpsData);



void register_TMobile();



#endif //VCU_FIRMWARE_2024_CELLULAR_H
