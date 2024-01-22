#include <cstring>
#include "cellular.h"

// private helper methods
static void cellular_send(std::string *command) {
  auto bytes = reinterpret_cast<const uint8_t *>(command->c_str());
  uint32_t error = HAL_UART_Transmit(&huart7, bytes, command->size(), 1000);
  if (error != HAL_OK) {
    Error_Handler();
  }
}

static void cellular_receive(std::string &expected) {
  static char buffer[512];
  memset(buffer, 0, sizeof(buffer));
  HAL_UART_Receive(&huart7, (uint8_t *) buffer, expected.size(), 5000);
  auto str = new std::string(buffer);
  if (*str != expected) {
    Error_Handler();
  }
}

static void cellular_receive_response(int size, std::string &response, int time) {
    static char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    HAL_UART_Receive(&huart7, (uint8_t *) buffer, size, time);
    response = std::string(buffer);


}

static void cellular_sendAndExpectOk(std::string *command) {
  cellular_send(command);
  cellular_receive(CELL_OK);
}

static bool cellular_areParametersUpdated() {
  return false;
}

static void cellular_sendTelemetry(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                   PduStatus *pduStatus, InverterStatus *inverterStatus,
                                   AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                                   ImuData *imuData, GpsData *gpsData) {

}

static void cellular_updateParameters(VcuParameters *vcuCoreParameters) {

}

static void cellular_disableEcho() {
  std::string command;

  command = "ATE0\r";
  cellular_sendAndExpectOk(&command); // TODO this will fail first time because it echoes the first time lol
}

static void cellular_setBaudRate() {
  std::string command;

  command = "AT+IPR=115200\r";
  cellular_sendAndExpectOk(&command);
}

static void cellular_testConnection() {
  std::string command;

  command = "AT\r";
  cellular_sendAndExpectOk(&command); // TODO don't fail, try again
}

static void cellular_mqttInit() {
  std::string command;
  std::string response;

  command = "AT+UMQTT=0,\"Car\"\r";
  response = "\r\r\n+UMQTT: 0,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response);

  command = "AT+UMQTT=1,1883\r";
  response = "\r\r\n+UMQTT: 1,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response);

  command = "AT+UMQTT=2,\"ec2-18-222-169-42.us-east-2.compute.amazonaws.com\",1883\r";
  response = "\r\r\n+UMQTT: 2,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response);

  command = "AT+UMQTT=10,60\r";
  response = "\r\r\n+UMQTT: 10,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response);

  command = "AT+UMQTT=12,1\r";
  response = "\r\r\n+UMQTT: 12,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response);

  command = "AT+UMQTTC=1\r";
  response = "\r\r\n+UMQTTC: 1,1\r\r\n\r\nOK\r\n\r\r\n+UUMQTTC: 1,0\r\r\n";
  cellular_send(&command);
  cellular_receive(response);
}

static void cellular_findTMobileHSNCode(std::string& code)
{

    /*
     Response example:
     "\r\n+COPS: (1,\"Verizon\",\"Verizon\",\"311480\",7),
     (1,\"313 100\",\"313 100\",\"313100\",7),(1,\"T-Mobile\",\"T-Mobile\",\"310260\",7),
     (1,\"311 490\",\"311 490\",\"311490\",7),(2,\"AT&T\",\"AT&T\",\"310410\",7),
     (1,\"T-Mobile\",\"T-Mobile\",\"310260\",9),,(0,1,2,3,4),(0,1,2)\r\n\r\nOK\r\n"
     */



    std::string command = "AT+COPS=?\r";
    cellular_send(&command);
    std::string response;
    cellular_receive_response(500, response, 180000);
    for(int i = 0; i < response.size(); i++)
    {
        char currentChar = response[i];
        if (currentChar == 'T')
        {
            char nextChar = response[i+1];
            char afterChar = response[i+2];
            if ((nextChar == '-') & (afterChar == 'M'))
            {
                i = i + 2;
                int initialP = 0;
                int endP = 0;
                currentChar = response[i];
                while(!(std::isdigit(currentChar)))
                {
                    i++;
                    currentChar = response[i];
                }
                currentChar = response[i];
                initialP = i;
                while((std::isdigit(currentChar)))
                {
                    i++;
                    currentChar = response[i];
                }
                endP = i;
                std::string hsn = "";
                for(int j = initialP; j < endP; j++)
                {
                    hsn = hsn + response[j];
                }
                code = hsn;
                break;


            }

        }
    }
}

// public methods
void cellular_init() {
  cellular_disableEcho();
//  cellular_testConnection();
//  cellular_mqttInit(); // TODO MQTT disconnect if already connected
//
//  std::string command = "AT+UMQTTC=2,0,0,\"/data/dynamics\",\"{'time': 1705190941.0, 'torque_command': 230}\"\r";
////  std::string command = "AT+UMQTTC=2,0,0,\"/data/dynamics\",\"{'time': 1705190941.0179203, 'frw_acc': [43.778969421635296, 43.65567322667927, 25.58023562609344, 66.16830247441165, 75.9495162863557], 'flw_acc': [37.50064131092476, 53.088920770080904, 47.227332043374915, 33.65606932318609, 41.536958453891515], 'brw_acc': [76.08696336342568, 80.5862624679301, 51.267235529547875, 49.03676806384779, 7.156970858460454], 'blw_acc': [97.95371431178432, 72.19636513295441, 58.71962346738052, 79.87786246683241, 58.10049539483053], 'body1_acc': [95.73636151406592, 65.0129876898297, 53.55348125950451, 38.730577656351606, 74.37797164406362], 'body1_ang': [94.97872961749091, 89.37709860274056, 72.14775543241576, 63.94064784462338, 39.2520389767362], 'body2_ang': [62.42865663116187, 56.240885674890464, 83.85965983972035, 14.26266907512983, 91.82640675058784], 'body2_acc': [27.589977016351273, 0.3363229086688957, 89.89544651582911, 54.65334676250442, 38.78303307430626], 'body3_acc': [37.94450174313836, 57.17031104688361, 98.61100576115857, 33.17994294388448, 75.5643466842868], 'body3_ang': [59.18938405408439, 22.667583995613427, 31.89591553798702, 59.25484100765107, 99.42443342277146], 'accel_pedal_pos': 81.27908789315963, 'brake_pressure': 74.71579823417692, 'motor_rpm': 19570, 'torque_command': 5564, 'gps': [-68.26350888272928, -84.21257804195574]}\"\r";
//  std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";
//  cellular_send(&command);
//  cellular_receive(response);

    register_TMobile();
}



void register_TMobile()
{
    std::string TMobile_HSNCode;
    cellular_findTMobileHSNCode(TMobile_HSNCode);
    std::string command = "AT+COPS=1,2,\"" + TMobile_HSNCode + "\"" + "\r";

    std::string response;
    cellular_send(&command);
    cellular_receive_response(500, response, 10000);
    command = "AT+COPS?\r";
    cellular_send(&command);
    cellular_receive_response(500, response, 10000);



}


void cellular_periodic(VcuParameters *vcuCoreParameters,
                       VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                       ImuData *imuData, GpsData *gpsData) {

  if (cellular_areParametersUpdated()) {
    cellular_updateParameters(vcuCoreParameters);
  }

  // TODO implement AT command stuff
}
