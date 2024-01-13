#include "cellular.h"

// private helper methods
static void cellular_sendAT(std::string* command) {
  auto bytes = reinterpret_cast<const uint8_t *>(command->c_str());
  uint32_t error = HAL_UART_Transmit(&huart7, bytes, command->size(), 1000);
  if(error != HAL_OK) {
    Error_Handler();
  }
}

static char buffer[256];
static std::string* cellular_receive() {
  buffer[0] = 0;
  HAL_UART_Receive(&huart7, (uint8_t*) buffer, 256, 50);
  auto str = new std::string(buffer);
  return str;
}

static bool cellular_areParametersUpdated() {
  return false;
}

static void cellular_sendTelemetry(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                   PduStatus *pduStatus, InverterStatus *inverterStatus,
                                   AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                                   ImuData *imuData, GpsData *gpsData) {
//  "+UMQTT=0,1,\"Car\""
//  "+UMQTT=1,1883"
//  "+UMQTT=2,\"ec2-3-138-110-121.us-east-2.compute.amazonaws.com\",1883"
//  "+UMQTT=10,60"
//  "+UMQTT=12,1"
//  "+UMQTTC=1"
//  wait then send data
//  "+UMQTTC=2,0,0,\"/data/trash\",\"Silly Goose\""
}

static void cellular_updateParameters(VcuParameters *vcuCoreParameters) {

}


// public methods
void cellular_init() {
//  std::string command = "AT\r";
  std::string command;
  std::string* response;

  command = "ATE0\r";
  cellular_sendAT(&command);
  response = cellular_receive();
  if(*response != "\r\nOK\r\n") {
    Error_Handler();
  }

  HAL_Delay(10);

  command = "AT\r";
  cellular_sendAT(&command);
  response = cellular_receive();
  if(*response != "\r\nOK\r\n") {
    Error_Handler();
  }
}

void cellular_periodic(VcuParameters *vcuCoreParameters,
                       VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                       ImuData *imuData, GpsData *gpsData) {

  if(cellular_areParametersUpdated()) {
    cellular_updateParameters(vcuCoreParameters);
  }

  // TODO implement AT command stuff
}
