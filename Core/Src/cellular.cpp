#include <cstring>
#include "cellular.h"
#include <queue>
#include "faults.h"
#include "secrets.h"


// private helper methods
static int cellular_send(std::string *command) {
  auto bytes = reinterpret_cast<const uint8_t *>(command->c_str());
  uint32_t error = HAL_UART_Transmit(&huart7, bytes, command->size(), HAL_MAX_DELAY);
  if (error != HAL_OK) {
    return error;
  }
  return 0;

}

static char commandBuffer[1024] = {0};
static bool finished_tx = true;
// Command queue, true is HF, false is LF, max size 50
std::queue<std::string> cellular_dataToSend;

static void cellular_sendNonBlocking(std::string &command) {
  if (!finished_tx) return;
  finished_tx = false;
  auto bytes = reinterpret_cast<const uint8_t *>(command.c_str());
  uint32_t error = HAL_UART_Transmit_DMA(&huart7, bytes, command.size());
  if (error != HAL_OK) {
    Error_Handler();
  }

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  finished_tx = true;
}

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static void cellular_split8(uint8_t *&arr, uint8_t num) {
  uint8_t *arr8 = reinterpret_cast<uint8_t *>(arr);
  *arr8 = num;
  arr += 1;
}

static void cellular_split16(uint8_t *&arr, uint16_t num) {
  uint16_t *arr16 = reinterpret_cast<uint16_t *>(arr);
  *arr16 = num;
  arr += 2;
}

static void cellular_split32(uint8_t *&arr, uint32_t num) {
  uint32_t *arr32 = reinterpret_cast<uint32_t *>(arr);
  *arr32 = num;
  arr += 4;
}


static int cellular_Base64encode(char *encoded, const uint8_t *string, int len) {
  int i;
  char *p;

  p = encoded;
  for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                    ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
  }
  if (i < len) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
      *p++ = basis_64[((string[i] & 0x3) << 4)];
      *p++ = '=';
    } else {
      *p++ = basis_64[((string[i] & 0x3) << 4) |
                      ((int) (string[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  *p++ = '\0';
  return p - encoded;
}


static bool cellular_receive(std::string &expected, bool care, uint32_t timeout) {
  static char buffer[512];
  memset(buffer, 0, sizeof(buffer));
  HAL_UART_Receive(&huart7, (uint8_t *) buffer, expected.size(), timeout);
  volatile auto str = new std::string(buffer);
  if (*str != expected) {
      volatile int x = 0;
      x++;
    return false;
  }
  return true;
}

static void cellular_receiveAny(int size, std::string &response, int time) {
  static char buffer[512];
  memset(buffer, 0, sizeof(buffer));
  uint16_t rxAmount = 0;
  HAL_UARTEx_ReceiveToIdle(&huart7, (uint8_t *) buffer, 2, &rxAmount, time); // \r\n
  HAL_UARTEx_ReceiveToIdle(&huart7, (uint8_t *) (buffer + 2), size - 2, &rxAmount, time);
  response = std::string(buffer);
}

static bool cellular_receiveNonBlocking(std::string &expectedResponse, std::string &response) {
  if (!cell_completeLine) {
    return false;
  }
  response = std::string(reinterpret_cast<const char *>(cell_completeLine));
  if (response != expectedResponse) {
    return false;
  }
  cell_completeLine = false;

}

static int cellular_sendAndExpectOk(std::string *command) {
  int success = cellular_send(command);
  success += cellular_receive(CELL_OK, true, 500);
  return success;
}

static bool cellular_areParametersUpdated() {
  return false;
}

// 0 = Did not Power On,
// 1 = Powered On but no connection
// 2 = Need to search for TMOBILE
// 3 = Connect to TMOBILE but not server
// 4 = Connected to server but did not send handshake
// 5 = sent handshake, running as normal
static uint8_t cellular_systemState = 0;

static void cellular_sendStartTime(GpsData *gpsData) {
  if (gpsData->year == 0) {
    return;
  }
  int data = gpsData->month;
  std::string date;
  if (data < 10) {
    date = "0" + std::to_string(data);
  } else {
    date = std::to_string(data);
  }

  std::string time = "20" + std::to_string(gpsData->year) + "-"
                     + date + "-";
  data = gpsData->day;
  if (data < 10) {
    date = "0" + std::to_string(data);
  } else {
    date = std::to_string(data);
  }
  time = time + date + "T";
  data = gpsData->hour;
  if (data < 10) {
    date = "0" + std::to_string(data);
  } else {
    date = std::to_string(data);
  }
  time = time + date + ":";
  data = gpsData->minute;
  if (data < 10) {
    date = "0" + std::to_string(data);
  } else {
    date = std::to_string(data);
  }
  time = time + date + ":";
  data = gpsData->seconds;
  if (data < 10) {
    date = "0" + std::to_string(data);
  } else {
    date = std::to_string(data);
  }
  data = gpsData->millis;
  time = time + date + "." + std::to_string(data);
  std::string command = "AT+UMQTTC=2,0,0,\"/config/car\",\"" + time + "\"\r";
  cellular_send(&command);
  cellular_systemState = 5;
}


static uint32_t cellular_calculateTimeDiff(GpsData *now) {

  // first in days
  int totalTime = 0;
  totalTime = (365) * (now->year - referenceGPSData.year);
  totalTime = totalTime + ((30) * (now->month - referenceGPSData.month));
  totalTime = totalTime + (now->day - referenceGPSData.day);
  // now hours
  totalTime = totalTime * 24;
  totalTime = totalTime + now->hour - referenceGPSData.hour;
  // now seconds
  totalTime = totalTime * 360;
  totalTime = totalTime + now->seconds - referenceGPSData.seconds + ((60) * (now->minute - referenceGPSData.minute));
  // now milli
  uint32_t millis = totalTime * 1000;
  millis = millis + now->millis - referenceGPSData.millis;
  return millis;

}

// change
static int time = 0;

static void cellular_sendTelemetryHigh(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                                       AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                                       ImuData *imuData, GpsData *gpsData) {


  std::string dataToEncode = "";
  // byte size of data
  uint8_t arr[143];
  uint8_t *ptr = arr;

  // For HF

  // version number
  uint8_t oneByteU = 1;
  cellular_split8(ptr, oneByteU);

  // Real Time Clock

  uint32_t fourBytesU = cellular_calculateTimeDiff(gpsData); // I do not know where to get this data
  cellular_split32(ptr, fourBytesU);


  // VCU TorqueRequest
  int16_t twoBytes = (int16_t) (vcuCoreOutput->inverterTorqueRequest / 0.1f);
  cellular_split16(ptr, twoBytes);

  // vcu flag
  uint16_t twoBytesU = 190; // I do not know where to get this data
  cellular_split16(ptr, twoBytesU);

  // vcu displacement x y z
  twoBytes = (int16_t) (vcuCoreOutput->vehicleDisplacement.x / 0.1f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleDisplacement.y / 0.1f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleDisplacement.z / 0.1f);
  cellular_split16(ptr, twoBytes);

  // VCU Velocity x y z
  twoBytes = (int16_t) (vcuCoreOutput->vehicleVelocity.x / 0.01f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleVelocity.y / 0.01f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleVelocity.z / 0.01f);
  cellular_split16(ptr, twoBytes);

  // VCU Acceleration
  twoBytes = (int16_t) (vcuCoreOutput->vehicleAcceleration.x / 0.001f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleAcceleration.y / 0.001f);
  cellular_split16(ptr, twoBytes);

  twoBytes = (int16_t) (vcuCoreOutput->vehicleAcceleration.z / 0.001f);
  cellular_split16(ptr, twoBytes);


  // HV Pack voltage TEMPORARY CHANGED
  twoBytesU = (uint16_t) (hvcStatus->packVoltage / 0.01f);
  cellular_split16(ptr, twoBytesU);


  ///////////
  // HV Tractive Voltage
  twoBytesU = (uint16_t) (0.01); // Dont know :(
  cellular_split16(ptr, twoBytesU);

  // HV Current
  twoBytes = (int16_t) (hvcStatus->packCurrent / 0.01f);
  cellular_split16(ptr, twoBytes);

  // LV Voltage
  twoBytesU = (uint16_t) (pduStatus->lvVoltage / 0.01f);
  cellular_split16(ptr, twoBytesU);

  // LV Current
  twoBytes = (int16_t) (pduStatus->lvCurrent / 0.01f);
  cellular_split16(ptr, twoBytes);

  // contactor state
  oneByteU = (uint8_t) (hvcStatus->contactorStatus);
  cellular_split8(ptr, oneByteU);

  // average cell voltage
  twoBytesU = 0; // dont know
  cellular_split16(ptr, twoBytesU);

  // average Cell Temp
  oneByteU = (uint8_t) (hvcStatus->packTempMean);
  cellular_split8(ptr, oneByteU);

  // apps 1
  twoBytesU = (uint16_t) (analogVoltages->apps1 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // apps 2
  twoBytesU = (uint16_t) (analogVoltages->apps2 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // bse1
  twoBytesU = (uint16_t) (analogVoltages->bse1 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // bse 2
  twoBytesU = (uint16_t) (analogVoltages->bse2 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // sus 1
  twoBytesU = (uint16_t) (analogVoltages->sus1 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // sus 2
  twoBytesU = (uint16_t) (analogVoltages->sus2 / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // steer
  twoBytesU = (uint16_t) (analogVoltages->steer / 0.001f);
  cellular_split16(ptr, twoBytesU);

  // gps Latitude
  int32_t fourBytes = (int32_t) ((gpsData->latitude / 0.0001f) * 60);
  cellular_split32(ptr, fourBytes);

  // gps longitude
  fourBytes = (int32_t) ((gpsData->longitude / 0.0001f) * 60);
  cellular_split32(ptr, fourBytes);

  // gps speed
  twoBytesU = (uint16_t) (gpsData->speed / 0.01f);
  cellular_split16(ptr, twoBytesU);

  // gps heading
  twoBytesU = (uint16_t) (gpsData->heading / 0.01f);
  cellular_split16(ptr, twoBytesU);



  // IMU Acceleration (x,y,z): accel1, accel2, accel3, Fl, Fr, Bl, Br
  twoBytes = (int16_t) (imuData->accelVcu.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelVcu.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelVcu.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelHvc.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelHvc.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelHvc.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelPdu.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelPdu.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelPdu.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFl.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFl.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFl.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFr.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFr.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelFr.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBl.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBl.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBl.z / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBr.x / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBr.y / 0.001f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->accelBr.z / 0.001f);
  cellular_split16(ptr, twoBytes);


  // IMU Gyro (x,y,z): gyro1, gyro2, gyro3
  twoBytes = (int16_t) (imuData->gyroVcu.x / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroVcu.y / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroVcu.z / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroHvc.x / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroHvc.y / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroHvc.z / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroPdu.x / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroPdu.y / 1);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (imuData->gyroPdu.z / 1);
  cellular_split16(ptr, twoBytes);


  // Wheel Speed: fl, fr, bl, br
  twoBytes = (int16_t) (wheelMagnetValues->fl / 0.1f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (wheelMagnetValues->fr / 0.1f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (wheelMagnetValues->bl / 0.1f);
  cellular_split16(ptr, twoBytes);
  twoBytes = (int16_t) (wheelMagnetValues->br / 0.1f);
  cellular_split16(ptr, twoBytes);

  // inverter Voltage
  twoBytesU = (uint16_t) (inverterStatus->voltage / 0.01f);
  cellular_split16(ptr, twoBytesU);

  // inverter current
  int16_t inverterCurrent = (int16_t) (inverterStatus->current / 0.01f);
  cellular_split16(ptr, inverterCurrent);

  // inverter RPM
  twoBytesU = (uint16_t) (inverterStatus->rpm / 1);
  cellular_split16(ptr, twoBytesU);

  // inverter Torque
  twoBytes = (int16_t) (inverterStatus->torqueActual / 0.1f);
  cellular_split16(ptr, twoBytes);


  // now encoding data
  char encoded[192];
  cellular_Base64encode(encoded, arr, 143);
  for (int i = 0; i < 192; i++) {
    dataToEncode = dataToEncode + encoded[i];
  }
  std::string command = "AT+UMQTTC=2,0,0,\"/h\",\"" + dataToEncode + "\"\r";
    cellular_dataToSend.push(command);
//  std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";\
//  std::string actual;
//  cellular_sendNonBlocking(command);
  // cellular_send(&command);
  // cellular_receiveAny(500, response, 10000);

}



static void cellular_sendTelemetryLow(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                      PduStatus *pduStatus, InverterStatus *inverterStatus,
                                      AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                                      ImuData *imuData, GpsData *gpsData) {
  std::string dataToEncode = "";
  uint8_t arr[404];
  uint8_t *ptr = arr;
  // Where do I get this data?

  // Where do I get this?
  // Getting millis :)

  uint8_t oneByteU = 1;
  cellular_split8(ptr, oneByteU);
  uint32_t uint32 = cellular_calculateTimeDiff(gpsData);
  cellular_split32(ptr, uint32);

  uint32 = 1;
  cellular_split32(ptr, uint32);

  uint32 = 1;
  cellular_split32(ptr, uint32);

  uint16_t cellVoltages[140];
  uint8_t cellTemperature[90];

  for (int i = 0; i < 140; i++) {
    cellVoltages[i] = 4;
    cellular_split16(ptr, cellVoltages[i]);
  }

  for (int i = 0; i < 90; i++) {
    cellTemperature[i] = 5;
    cellular_split8(ptr, cellTemperature[i]);
  }

  uint16_t hvSOC = (uint16_t) (hvcStatus->stateOfCharge / 0.01f);
  cellular_split16(ptr, hvSOC);

  uint16_t lvSOC = (uint16_t) (pduStatus->lvSoC / 0.01f);
  cellular_split16(ptr, lvSOC);

  uint8_t ambientTemp = 0; // dont know
  cellular_split8(ptr, ambientTemp);

  uint8_t inverterTemp = inverterStatus->inverterTemp;
  cellular_split8(ptr, inverterTemp);

  uint8_t motorTemp = inverterStatus->motorTemp;
  cellular_split8(ptr, motorTemp);

  uint8_t waterMotorTemp = pduStatus->waterTempMotor;
  cellular_split8(ptr, waterMotorTemp);

  uint8_t waterInverterTemp = pduStatus->waterTempInverter;
  cellular_split8(ptr, waterInverterTemp);

  uint8_t waterRadiatorTemp = pduStatus->waterTempRadiator;
  cellular_split8(ptr, waterRadiatorTemp);

  // need clarification on this
  uint16_t radFanSet = 0;
  cellular_split16(ptr, radFanSet);

  uint16_t radFanSpeed = 0;
  cellular_split16(ptr, radFanSpeed);

  uint16_t batFanSet = 0;
  cellular_split16(ptr, batFanSet);

  uint16_t batFanSpeed = 0;
  cellular_split16(ptr, batFanSpeed);

  uint16_t waterFlowRate = 0;
  cellular_split16(ptr, waterFlowRate);


  char encoded[540];
  cellular_Base64encode(encoded, arr, 404);
  for (int i = 0; i < 540; i++) {
    dataToEncode = dataToEncode + encoded[i];
  }
  std::string command = "AT+UMQTTC=2,0,0,\"/l\",\"" + dataToEncode + "\"\r";
//  std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";
//  cellular_sendNonBlocking(command);
    cellular_dataToSend.push(command);
  // cellular_receive(response, false, 1000);

}

static void cellular_updateParameters(VcuParameters *vcuCoreParameters) {

}

static void cellular_disableEcho() {
  std::string command;
  std::string response;

  command = "ATE0\r";
  cellular_send(&command); // TODO this will fail first time because it echoes the first time lol
  cellular_receive(CELL_OK, false, 500);
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
  cellular_receive(CELL_OK, false, 500);
}

static void cellular_mqttInit() {
  std::string command;
  std::string response;
  command = "AT+UMQTT=0,\"Car\"\r";
  response = "\r\r\n+UMQTT: 0,1\r\r\n\r\nOK\r\n";
  int error = 0;
    error = cellular_send(&command);
  if (error != 0)
  {
      return;
  }
  cellular_receiveAny(64, response, 1000);
  if (response.size() > 23) {
    return;
  }

  command = "AT+UMQTT=1,1883\r";
  response = "\r\r\n+UMQTT: 1,1\r\r\n\r\nOK\r\n";
    error = cellular_send(&command);
    if (error != 0)
    {
        return;
    }
  cellular_receive(response, true, 1000);

  command = std::string("AT+UMQTT=2,\"") + AWS_SERVER + std::string("\",1883\r");
  response = "\r\r\n+UMQTT: 2,1\r\r\n\r\nOK\r\n";
    error = cellular_send(&command);
    if (error != 0)
    {
        return;
    }
  cellular_receive(response, true, 1000);

  command = "AT+UMQTT=10,3600\r";
  response = "\r\r\n+UMQTT: 10,1\r\r\n\r\nOK\r\n";
    error = cellular_send(&command);
    if (error != 0)
    {
        return;
    }
  cellular_receive(response, true, 1000);

  command = "AT+UMQTT=12,1\r";
  response = "\r\r\n+UMQTT: 12,1\r\r\n\r\nOK\r\n";
    error = cellular_send(&command);
    if (error != 0)
    {
        return;
    }
  cellular_receive(response, true, 1000);

  command = "AT+UMQTTC=1\r";
  response = "\r\r\n+UMQTTC: 1,1\r\r\n\r\nOK\r\n\r\r\n+UUMQTTC: 1,0\r\r\n";
    error = cellular_send(&command);
    if (error != 0)
    {
        return;
    }
  bool good = cellular_receive(response, true, 5000);
    if(!good) {
        volatile int x = 0;
        x++;
        return;
    }
  cellular_systemState = 4;
}

static void cellular_findTMobileHSNCode(std::string &code, std::string &response) {

  /*
   Response example:
   "\r\n+COPS: (1,\"Verizon\",\"Verizon\",\"311480\",7),
   (1,\"313 100\",\"313 100\",\"313100\",7),(1,\"T-Mobile\",\"T-Mobile\",\"310260\",7),
   (1,\"311 490\",\"311 490\",\"311490\",7),(2,\"AT&T\",\"AT&T\",\"310410\",7),
   (1,\"T-Mobile\",\"T-Mobile\",\"310260\",9),,(0,1,2,3,4),(0,1,2)\r\n\r\nOK\r\n"
   */



  for (int i = 0; i < response.size(); i++) {
    char currentChar = response[i];
    if (currentChar == 'T') {
      char nextChar = response[i + 1];
      char afterChar = response[i + 2];
      if ((nextChar == '-') & (afterChar == 'M')) {
        i = i + 2;
        int initialP = 0;
        int endP = 0;
        currentChar = response[i];
        while (!(std::isdigit(currentChar))) {
          i++;
          currentChar = response[i];
        }
        currentChar = response[i];
        initialP = i;
        while ((std::isdigit(currentChar))) {
          i++;
          currentChar = response[i];
        }
        endP = i;
        std::string hsn = "";
        for (int j = initialP; j < endP; j++) {
          hsn = hsn + response[j];
        }
        code = hsn;
        break;


      }

    }
  }
}


static void cellular_registerTMobile() {
  std::string command = "AT+COPS?\r";
  std::string response;
  cellular_send(&command);
  cellular_receiveAny(500, response, 5000);
  for (int i = 0; i < response.size(); i++) {
    if (response[i] == 'T') {
        cellular_systemState = 3;
      return;
    }
  }
  if (cellular_systemState == 2) {
    std::string command = "AT+COPS=?\r";
    cellular_sendNonBlocking(command);
  }

}

static void cellular_subscribe() {
  std::string command = "AT+UMQTTC=4,0,\"#\"\r";

  cellular_send(&command);
  uint8_t buffer[256];
  memset(buffer, 0, 256);
  HAL_UART_Receive(&huart7, (uint8_t *) buffer, 256, 5000);

//    cellular_receiveAny(500, response, 10000);

}


static void cellular_poll() {
  std::string command = "AT+UMQTTC=6\r";
  std::string response;
  cellular_send(&command);
  uint8_t buffer[256];
  memset(buffer, 0, 256);
  HAL_UART_Receive(&huart7, (uint8_t *) buffer, 256, 500);
  std::string num_response = "";
  for (int i = 0; i < 256; i++) {

    uint8_t current = buffer[i];
    if (current == uint8_t('h')) {
      // TODO do something
    }
  }
}

extern VcuOutput vcuCoreOutput;
extern GpsData gpsData;

static void cellular_respondToText(std::string *senderPhoneNumber, std::string *message) {
  std::string latString;
  if (gpsData.latitude > 0) {
    latString = std::to_string(gpsData.latitude);
    latString += " N";
  } else {
    latString = std::to_string(-gpsData.latitude);
    latString += " S";
  }

  std::string longString;
  if (gpsData.longitude > 0) {
    longString = std::to_string(gpsData.longitude);
    longString += " E";
  } else {
    longString = std::to_string(-gpsData.longitude);
    longString += " W";
  }

  std::string response = "I'm parked at " + latString + ", " + longString + ".";
  if (vcuCoreOutput.prndlState) {
    response = "I'm driving right now. Talk to you later.";
  }
  cellular_sendText(senderPhoneNumber, &response);
}

// public methods
uint8_t rxbuffer[1024];

void cellular_init() {
  HAL_GPIO_WritePin(CELL_PWR_GPIO_Port, CELL_PWR_Pin, GPIO_PIN_RESET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(CELL_PWR_GPIO_Port, CELL_PWR_Pin, GPIO_PIN_SET);
//  HAL_Delay(8000);
//  cellular_disableEcho();
//    cellular_testConnection();
//    cellular_disableEcho();
//    cellular_disableEcho();
//    cellular_disableEcho();
//    cellular_testConnection();
//    cellular_registerTMobile();
//    if(hasConnection)
//    {
////        cellular_mqttInit();
//    }




// TODO MQTT disconnect if already connected
//////
//for(int i = 0; i < 200; i++)
//{
//    std::string command = "AT+UMQTTC=2,0,0,\"/data/dynamics\",\"{'time': " + std::to_string(1706483160 + i) + ", 'torque_command': " + std::to_string(i) + "}\"\r";
//    std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";
//    cellular_send(&command);
//    cellular_receive(response, false, 1000);
//    HAL_Delay(5000);
//
//}

//  std::string phoneNumber = "18326411809";
//  std::string message = "Hello Matthew, we are here to talk about your car's extended warranty!";
//  cellular_sendText(&phoneNumber, &message);





}


int cellular_sendText(std::string *phoneNumber, std::string *message) {
  std::string command;
  std::string response;

  command = "AT+CMGF=1\r";
  int success = cellular_sendAndExpectOk(&command);

  command = "AT+CMGS=\"" + *phoneNumber + "\"\r";
  response = "\r\n> ";
  cellular_send(&command);
  cellular_receive(response, true, 5000);

  cellular_send(message);

  command = "\x1A"; // Ctrl+Z character to indicate end of message
  cellular_send(&command);

  cellular_receiveAny(64, response, 30000);
  uint8_t index = response.size() - 6;
  const char *okBegin = response.c_str() + index;
  if (strcmp(okBegin, "\r\nOK\r\n") != 0) {
    return 1;
  }
  return 0;
}

void cellular_respondToTexts() {
  std::string command;
  std::string response;
  std::string sender;
  std::string message;

    HAL_UART_DMAPause(&huart7);
  command = "AT+CMGF=1\r";
  int success = cellular_sendAndExpectOk(&command);

  command = "AT+CMGL\r";
  cellular_send(&command);
  cellular_receiveAny(2048, response, 1000);

  // delete read messages to save space
  command = "AT+CMGD=0,3\r";
  cellular_sendAndExpectOk(&command);

  for (int i = 0; i < response.size();) {
    i += 2;
    if (i >= response.size() || (response.at(i) == 'O' && response.at(i + 1) == 'K')) {
      break;
    }
    if (response.at(i) != '+') {
      break;
    }
    int commas = 0;
    for (; i < response.size(); i++) {
      if (response.at(i) == ',') {
        commas++;
        if (commas == 2) {
          break;
        }
      }
    }
    i += 2;
    if (i >= response.size()) {
      break;
    }
    sender = "";
    for (; i < response.size(); i++) {
      if (response.at(i) == '\"') {
        break;
      }
      sender += response.at(i);
    }
    if (i >= response.size()) {
      break;
    }
    for (; i < response.size(); i++) {
      if (response.at(i) == '\n') {
        break;
      }
    }
    i += 1;
    if (i >= response.size()) {
      break;
    }
    message = "";
    for (; i < response.size(); i++) {
      if (response.at(i) == '\r') {
        break;
      }
      message += response.at(i);
    }
    if (i >= response.size()) {
      break;
    }

    cellular_respondToText(&sender, &message);
    HAL_UART_DMAResume(&huart7);
  }
}





void cellular_periodic(VcuParameters *vcuCoreParameters,
                       VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                       AnalogVoltages *analogVoltages, WheelMagnetValues *wheelMagnetValues,
                       ImuData *imuData, GpsData *gpsData) {

  /*
   * if(not_connected) {
   * if(cell_complete_line) string urmom = cell_currline;
   * else return;
   * TMOBILE_idx = urmom.contains(TMOBILE)
   * if(TMOBILE_idx != npos) {
   * val_we_want urmom.substr(TMOBILE_idx, 10)
   * not_connected = true;
   * }
   * else return;
   * }
   */

  // did not power on!
  if(cellular_systemState == 0)
  {
      float timeSinceStart = clock_getTime();
      if (timeSinceStart < 12.0f)
      {
          return;
      }
      else
      {
          // Powered ON! Now Starting up cellular
          cellular_disableEcho();
          cellular_testConnection();
          cellular_disableEcho();
          cellular_disableEcho();
          cellular_disableEcho();
          cellular_testConnection();
          volatile uint32_t error = HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *) cell_tempLine, MAX_CELL_LINE_SIZE);
          error = error + 0;
          cellular_systemState = 1;
          // requesting current connection information
          std::string command = "AT+COPS?\r";
          cellular_send(&command);

      }
  }
  // waiting to check current connection info
  else if (cellular_systemState == 1)
  {
      if (cell_completeLine)
      {
          std::string response = cell_currLine;
          for (int i = 0; i < response.size(); i++) {
              if (response[i] == 'T') {
                  cellular_systemState = 3;
                  return;
              }
          }
          if(cellular_systemState == 1)
          {
              cellular_systemState = 2;
              std::string command = "AT+COPS=?\r";
              cellular_sendNonBlocking(command);
          }
      }

  }
  // powered on but no connection
  else if (cellular_systemState == 2)
  {
    if (cell_completeLine)
    {
      std::string availableConns = cell_currLine;
      std::string TMobile_HSNCode = "0";
      cellular_findTMobileHSNCode(TMobile_HSNCode, availableConns);
      if (TMobile_HSNCode != "0")
      {
        std::string command = "AT+COPS=1,2,\"" + TMobile_HSNCode + "\"" + "\r";
        cellular_send(&command);
        cellular_systemState = 3;
      }
    }
    static float lastSearchingTime = 0;
    float currentSearchingTime = clock_getTime();
    if (currentSearchingTime - lastSearchingTime > 180.0f) {
      lastSearchingTime = currentSearchingTime;
      std::string command = "AT+COPS=?\r";
      cellular_sendNonBlocking(command);

    }
  }
  // has connection but no server = 3
  else if (cellular_systemState == 3)
  {
      cellular_mqttInit();
  }
  else
  {
      // when we are in Park
      if(!vcuCoreOutput->prndlState == false)
      {
          cellular_systemState = 4;
          cellular_subscribe();
            while(vcuCoreOutput->prndlState == false)
            {
                cellular_poll();
            }
      }
      // when we are driving
      else
      {
          // Check if we made the handshake
          // State = 4
          if (cellular_systemState == 4)
          {
              cellular_sendStartTime(gpsData);
          }
          // if handshake has been made
          // State = 5
          else
          {
              if (cellular_areParametersUpdated())
              {
                  cellular_updateParameters(vcuCoreParameters);
              }
              // send H/L freq messages
              else
              {
                  static float lastHFTime = 0;
                  float nowHFTime = clock_getTime();
                  // should be 0.05
                  if (nowHFTime - lastHFTime > 0.05f) {
                      lastHFTime = nowHFTime;
                      if (cellular_dataToSend.size() < 10)
                      {
                          cellular_sendTelemetryHigh(vcuCoreOutput, hvcStatus,
                                                    pduStatus, inverterStatus,
                                                    analogVoltages, wheelMagnetValues,
                                                    imuData, gpsData);
                      }
                      else
                      {

                      }
                  }
                  static float lastLFTime = 0;
                  float nowLFTime = clock_getTime();
                  if (nowLFTime - lastLFTime > 1.0f) {
                      lastLFTime = nowLFTime;
                      if (cellular_dataToSend.size() < 10)
                      {
                          cellular_sendTelemetryLow(vcuCoreOutput, hvcStatus,
                                                    pduStatus, inverterStatus,
                                                    analogVoltages, wheelMagnetValues,
                                                    imuData, gpsData);
                      }
                      else
                      {

                      }
                      time++;
                  }
                  static float lastSeconds = 0;
                  float nowSeconds = clock_getTime();
                  if (nowSeconds - lastSeconds > 1.0f) {
                      lastSeconds = nowSeconds;
                      time++;
                  }
                  if(finished_tx)
                  {
                      std::string command = cellular_dataToSend.front();
                      cellular_sendNonBlocking(command);
                      cellular_dataToSend.pop();
                  }
              }
//              // check and respond to text messages once per second
//              static float lastTextTime = 0;
//              float nowTextTime = clock_getTime();
//              if (nowTextTime - lastTextTime > 1.0f) {
//                  lastTextTime = nowTextTime;
//                  cellular_respondToTexts();
//              }
          }
      }

  }
}
