#include <cstring>
#include "cellular.h"

// private helper methods
static void cellular_send(std::string *command) {
  auto bytes = reinterpret_cast<const uint8_t *>(command->c_str());
  uint32_t error = HAL_UART_Transmit(&huart7, bytes, command->size(), HAL_MAX_DELAY);
  if (error != HAL_OK) {
    Error_Handler();
  }
}

static bool cellular_receive(std::string &expected, bool care) {
  static char buffer[512];
  memset(buffer, 0, sizeof(buffer));
  HAL_UART_Receive(&huart7, (uint8_t *) buffer, expected.size(), 30000);
  auto str = new std::string(buffer);
  if (*str != expected) {
      if(care) {
          Error_Handler();
      }else {
          return false;
      }
  }
  return true;
}

static void cellular_receiveAny(int size, std::string &response, int time) {
    static char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    uint16_t rxAmount = 0;
    HAL_UARTEx_ReceiveToIdle(&huart7, (uint8_t *) buffer, 2, &rxAmount, time); // \r\n
    HAL_UARTEx_ReceiveToIdle(&huart7, (uint8_t *) buffer, size, &rxAmount, time);
    response = "\r\n" + std::string(buffer);
}

static void cellular_sendAndExpectOk(std::string *command) {
  cellular_send(command);
  cellular_receive(CELL_OK, true);
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
  std::string response;

  command = "ATE0\r";
  cellular_send(&command); // TODO this will fail first time because it echoes the first time lol
  cellular_receive(CELL_OK, false);
}

static void cellular_setBaudRate() {
  std::string command;

  command = "AT+IPR=115200\r";
  cellular_sendAndExpectOk(&command);
}

static void cellular_testConnection() {
  std::string command;

  command = "AT\r";
//  cellular_sendAndExpectOk(&command); // TODO don't fail, try again
    cellular_send(&command);
    cellular_receive(CELL_OK, false);
}

static void cellular_mqttInit() {
  std::string command;
  std::string response;
  volatile int y = 10;
  command = "AT+UMQTT=0,\"Car\"\r";
  response = "\r\r\n+UMQTT: 0,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  bool success = cellular_receive(response, false);
  if (!success)
  {
      return;
  }
  y++;
  command = "AT+UMQTT=1,1883\r";
  response = "\r\r\n+UMQTT: 1,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true);
    y++;
  command = "AT+UMQTT=2,\"ec2-3-19-28-149.us-east-2.compute.amazonaws.com\",1883\r";
  response = "\r\r\n+UMQTT: 2,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true);
    y++;
  command = "AT+UMQTT=10,60\r";
  response = "\r\r\n+UMQTT: 10,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true);
    y++;
  command = "AT+UMQTT=12,1\r";
  response = "\r\r\n+UMQTT: 12,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true);
    y++;
  command = "AT+UMQTTC=1\r";
  response = "\r\r\n+UMQTTC: 1,1\r\r\n\r\nOK\r\n\r\r\n+UUMQTTC: 1,0\r\r\n";
  cellular_send(&command);
  cellular_receive(response, true);
    y++;
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
  cellular_receiveAny(500, response, 180000);
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

static void cellular_sendText(std::string* phoneNumber, std::string* message) {
  std::string command;
  std::string response;

  command = "AT+CMGF=1\r";
  cellular_sendAndExpectOk(&command);

  command = "AT+CMGS=\"" + *phoneNumber + "\"\r";
  response = "\r\n> ";
  cellular_send(&command);
  cellular_receive(response, true);

  cellular_send(message);

  command = "\x1A"; // Ctrl+Z character to indicate end of message
  response = "\r\n\r\n+CMGS: 3\r\n\r\nOK\r\n";
  cellular_send(&command);

  cellular_receiveAny(64, response, 30000);
  uint8_t index = response.size() - 6;
  const char* okBegin = response.c_str() + index;
  if(strcmp(okBegin, "\r\nOK\r\n") != 0) {
    Error_Handler();
  }
}

// public methods
void cellular_init() {
  cellular_disableEcho();
  cellular_disableEcho();
  cellular_disableEcho();
  cellular_disableEcho();
  cellular_testConnection();
  register_TMobile();

  std::string phoneNumber = "15555555555";
  std::string message = "Hi it's Angelique ;)";
  cellular_sendText(&phoneNumber, &message);


//  cellular_mqttInit(); // TODO MQTT disconnect if already connected
//////
//for(int i = 0; i < 200; i++)
//{
//    std::string command = "AT+UMQTTC=2,0,0,\"/data/dynamics\",\"{'time': " + std::to_string(1706483160 + i) + ", 'torque_command': " + std::to_string(i) + "}\"\r";
//    std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";
//    cellular_send(&command);
//    cellular_receive(response, false);
//    HAL_Delay(5000);
//
//}



}



void register_TMobile()
{
    std::string command = "AT+COPS?\r";
    std::string response;
    cellular_send(&command);
  cellular_receiveAny(500, response, 1000);
    bool has_conn = false;
    for(int i = 0; i < response.size(); i++)
    {
        if (response[i] == 'T')
        {
            has_conn = true;
        }
    }
    if(not has_conn)
    {
        std::string TMobile_HSNCode;
        cellular_findTMobileHSNCode(TMobile_HSNCode);

        command = "AT+COPS=1,2,\"" + TMobile_HSNCode + "\"" + "\r";

        cellular_send(&command);
      cellular_receiveAny(500, response, 180000);
    }

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
