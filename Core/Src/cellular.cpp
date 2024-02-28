#include <cstring>
#include "cellular.h"



// private helper methods
static void cellular_send(std::string *command) {
  auto bytes = reinterpret_cast<const uint8_t *>(command->c_str());
  volatile int x = 0;
  uint32_t error = HAL_UART_Transmit(&huart7, bytes, command->size(), HAL_MAX_DELAY);
  if (error != HAL_OK) {
    Error_Handler();
  }
  x++;
}

static char commandBuffer[1024] = {0};
static bool finished_tx = true;
static void cellular_sendNonBlocking(std::string& command){
    if(!finished_tx) return;
    finished_tx = false;
    auto bytes = reinterpret_cast<const uint8_t *>(command.c_str());
    uint32_t error = HAL_UART_Transmit_DMA(&huart7, bytes, command.size());
    if(error != HAL_OK){
        Error_Handler();
    }

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart){
    finished_tx = true;
}

static const char basis_64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static void cellular_split8(uint8_t*& arr, uint8_t num)
{
    uint8_t* arr8 = reinterpret_cast<uint8_t *>(arr);
    *arr8 = num;
    arr += 1;
}
static void cellular_split16(uint8_t*& arr, uint16_t num)
{
    uint16_t* arr16 = reinterpret_cast<uint16_t *>(arr);
    *arr16 = num;
    arr += 2;
}
static void cellular_split32(uint8_t*& arr, uint32_t num)
{
    uint32_t* arr32 = reinterpret_cast<uint32_t *>(arr);
    *arr32 = num;
    arr += 4;
}


static int cellular_Base64encode(char *encoded, const uint8_t *string, int len)
{
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
        }
        else {
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
    HAL_UARTEx_ReceiveToIdle(&huart7, (uint8_t *) (buffer+2), size-2, &rxAmount, time);
    response = std::string(buffer);
}

static bool cellular_receiveNonBlocking(std::string& expectedResponse, std::string& response){
    if(!cell_completeLine){
        return false;
    }
    response = std::string(reinterpret_cast<const char *>(cell_completeLine));
    if (response != expectedResponse) {
        return false;
    }
    cell_completeLine = false;

}

static void cellular_sendAndExpectOk(std::string *command) {
  cellular_send(command);
  cellular_receive(CELL_OK, true, 500);
}

static bool cellular_areParametersUpdated() {
  return false;
}

static bool startTimeSent = false;

static void cellular_sendStartTime(GpsData *gpsData)
{
    if(gpsData->year == 0)
    {
        return;
    }
    startTimeSent = true;
    int data = gpsData->month;
    std::string date;
    if(data < 10)
    {
        date = "0" + std::to_string(data);
    }
    else
    {
        date = std::to_string(data);
    }

    std::string time = "20" + std::to_string(gpsData->year) + "-"
                       + date + "-";
    data = gpsData->day;
    if(data < 10)
    {
        date = "0" + std::to_string(data);
    }
    else
    {
        date = std::to_string(data);
    }
    time = time + date + "T";
    data = gpsData->hour;
    if(data < 10)
    {
        date = "0" + std::to_string(data);
    }
    else
    {
        date = std::to_string(data);
    }
    time = time + date + ":";
    data = gpsData->minute;
    if(data < 10)
    {
        date = "0" + std::to_string(data);
    }
    else
    {
        date = std::to_string(data);
    }
    time = time + date + ":";
    data = gpsData->seconds;
    if(data < 10)
    {
        date = "0" + std::to_string(data);
    }
    else
    {
        date = std::to_string(data);
    }
    data = gpsData->millis;
    time = time + date + "." + std::to_string(data);
    std::string command = "AT+UMQTTC=2,0,0,\"/config/car\",\"" + time + "\"\r";
    volatile int r = 1100;
    cellular_send(&command);
}

static void cellular_sendTelemetryHigh(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                   PduStatus *pduStatus, InverterStatus *inverterStatus,
                                   AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                                   ImuData *imuData, GpsData *gpsData)

   {


        std::string dataToEncode = "";
        // byte size of data
        uint8_t arr[143];
        uint8_t *ptr = arr;

        // For HF

        // version number
        uint8_t oneByteU = 1;
        cellular_split8(ptr, oneByteU);

        // Real Time Clock
        uint32_t fourBytesU = 0; // I do not know where to get this data
        cellular_split32(ptr, fourBytesU);

        // VCU TorqueRequest
        int16_t twoBytes = (int16_t)(vcuCoreOutput->inverterTorqueRequest / 0.1f);
        cellular_split16(ptr, twoBytes);

        // vcu flag
        uint16_t twoByesU = 0; // I do not know where to get this data
        cellular_split16(ptr, twoBytes);

        // vcu displacement x y z
        twoBytes = (int16_t)(vcuCoreOutput->vehicleDisplacement.x / 0.1f);
        cellular_split16(ptr, twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleDisplacement.y / 0.1f);
        cellular_split16(ptr, twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleDisplacement.z / 0.1f);
        cellular_split16(ptr, twoBytes);

        // VCU Velocity x y z
        twoBytes = (int16_t)(vcuCoreOutput->vehicleVelocity.x / 0.01f);
        cellular_split16(ptr, twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleVelocity.y / 0.01f);
        cellular_split16(ptr,twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleVelocity.z / 0.01f);
        cellular_split16(ptr, twoBytes);

        // VCU Acceleration
        twoBytes = (int16_t)(vcuCoreOutput->vehicleAcceleration.x / 0.001f);
        cellular_split16(ptr, twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleAcceleration.y / 0.001f);
        cellular_split16(ptr, twoBytes);

        twoBytes = (int16_t)(vcuCoreOutput->vehicleAcceleration.z / 0.001f);
        cellular_split16(ptr, twoBytes);


        // HV Pack voltage
        twoByesU = (uint16_t)(hvcStatus->packVoltage / 0.01f);
        cellular_split16(ptr, twoByesU);

        // HV Tractive Voltage
        twoByesU = (uint16_t)(0.01); // Dont know :(
        cellular_split16(ptr, twoByesU);

        // HV Current
        twoBytes = (int16_t)(hvcStatus->packCurrent / 0.01f);
        cellular_split16(ptr, twoBytes);

        // LV Voltage
        twoByesU = (uint16_t)(pduStatus->lvVoltage / 0.01f);
        cellular_split16(ptr, twoByesU);

        // LV Current
        twoBytes = (int16_t)(pduStatus->lvCurrent / 0.01f);
        cellular_split16(ptr, twoBytes);

        // contactor state
        oneByteU = (uint8_t)(hvcStatus->contactorStatus);
        cellular_split8(ptr, oneByteU);

        // average cell voltage
        twoByesU = 0; // dont know
        cellular_split16(ptr, twoByesU);

        // average Cell Temp
        oneByteU = (uint8_t)(hvcStatus->packTempMean);
        cellular_split8(ptr, oneByteU);

        // apps 1
        twoByesU = (uint16_t)(analogVoltages->apps1 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // apps 2
        twoByesU = (uint16_t)(analogVoltages->apps2 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // bse1
        twoByesU = (uint16_t)(analogVoltages->bse1 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // bse 2
        twoByesU = (uint16_t)(analogVoltages->bse2 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // sus 1
        twoByesU = (uint16_t)(analogVoltages->sus1 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // sus 2
        twoByesU = (uint16_t)(analogVoltages->sus2 / 0.001f);
        cellular_split16(ptr, twoByesU);

        // steer
        twoByesU = (uint16_t)(analogVoltages->steer / 0.001f);
        cellular_split16(ptr, twoByesU);

        // gps Latitude
        int32_t fourBytes = (int32_t)(gpsData->latitude / 0.0001f);
        cellular_split32(ptr, fourBytes);

        // gps longitude
        fourBytes = (int32_t)(gpsData->longitude / 0.0001f);
        cellular_split32(ptr, fourBytes);

        // gps speed
        twoByesU = (uint16_t)(gpsData->speed / 0.01f);
        cellular_split16(ptr, twoByesU);

        // gps heading
        twoByesU = (uint16_t)(gpsData->heading / 0.01f);
        cellular_split16(ptr, twoByesU);



        // IMU Acceleration (x,y,z): accel1, accel2, accel3, Fl, Fr, Bl, Br
        twoBytes = (int16_t)(imuData->accelVcu.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelVcu.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelVcu.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelHvc.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelHvc.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelHvc.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelPdu.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelPdu.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelPdu.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFl.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFl.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFl.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFr.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFr.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelFr.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBl.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBl.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBl.z / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBr.x / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBr.y / 0.001f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->accelBr.z / 0.001f);
        cellular_split16(ptr, twoBytes);


        // IMU Gyro (x,y,z): gyro1, gyro2, gyro3
        twoBytes = (int16_t)(imuData->gyroVcu.x / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroVcu.y / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroVcu.z / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroHvc.x / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroHvc.y / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroHvc.z / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroPdu.x / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroPdu.y / 1);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(imuData->gyroPdu.z / 1);
        cellular_split16(ptr, twoBytes);


        // Wheel Speed: fl, fr, bl, br
        twoBytes = (int16_t)(wheelDisplacements->fl / 0.1f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(wheelDisplacements->fr / 0.1f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(wheelDisplacements->bl / 0.1f);
        cellular_split16(ptr, twoBytes);
        twoBytes = (int16_t)(wheelDisplacements->br / 0.1f);
        cellular_split16(ptr, twoBytes);

        // inverter Voltage
        twoByesU = (uint16_t)(inverterStatus->voltage / 0.01f);
        cellular_split16(ptr, twoByesU);

        // inverter current
        int16_t inverterCurrent = (int16_t)(inverterStatus->current / 0.01f);
        cellular_split16(ptr, inverterCurrent);

        // inverter RPM
        twoByesU = (uint16_t)(inverterStatus->rpm / 1);
        cellular_split16(ptr, twoByesU);

        // inverter Torque
        twoBytes = (int16_t)(inverterStatus->torqueActual / 0.1f);
        cellular_split16(ptr, twoBytes  );


        // now encoding data
        char encoded[192];
        cellular_Base64encode(encoded, arr, 143);
        for(int i = 0; i < 192; i++)
        {
            dataToEncode = dataToEncode + encoded[i];
        }
        std::string command = "AT+UMQTTC=2,0,0,\"/h\",\"" + dataToEncode + "\"\r";
        std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";\
        std::string actual;
        volatile int z = 0;
        cellular_sendNonBlocking(command);
        // cellular_send(&command);
        // cellular_receiveAny(500, response, 10000);
        z++;

   }

static void cellular_createDummyHFSend(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                                       ImuData *imuData, GpsData *gpsData)
{
    // For HF



    // VCU TorqueRequest
    vcuCoreOutput->inverterTorqueRequest = 19;

    vcuCoreOutput->vehicleDisplacement.x = 10;

    vcuCoreOutput->vehicleDisplacement.y = 9;

    vcuCoreOutput->vehicleDisplacement.z = -8;

    // VCU Velocity x y z
    vcuCoreOutput->vehicleVelocity.x = -31;


    vcuCoreOutput->vehicleVelocity.y = 91;

    vcuCoreOutput->vehicleVelocity.z = -19;

    // VCU Acceleration
    vcuCoreOutput->vehicleAcceleration.x = 1;


    vcuCoreOutput->vehicleAcceleration.y = 0.2f;

    vcuCoreOutput->vehicleAcceleration.z = 0.045f;



    // HV Pack voltage
    hvcStatus->packVoltage = 0.1f;


    // HV Current
    hvcStatus->packCurrent = 0.1f;

    // LV Voltage
    pduStatus->lvVoltage = 0.78f;

    // LV Current
    pduStatus->lvCurrent = -3.25f;


    // contactor state
    hvcStatus->contactorStatus = HvcStatus::FULLY_OPEN;


    // average Cell Temp
    hvcStatus->packTempMean = 0.1f;


    // apps 1
    analogVoltages->apps1 = 0.04f;

    // apps 2
    analogVoltages->apps2 = 1.2f;

    // bse1
    analogVoltages->bse1 = 3.2f;


    // bse 2
    analogVoltages->bse2 = 0.045f;

    // sus 1
    analogVoltages->sus1 = 0.03f;


    // sus 2
    analogVoltages->sus2 = 0.05f;


    // steer
    analogVoltages->steer = 0.45f;

    // gpd Latitude
    gpsData->latitude = -0.0543f;

    // gps longitude
    gpsData->longitude = 0.00456f;

    // gps speed
    gpsData->speed = 0.34f;

    // gps heading
    gpsData->heading = 1;




    // IMU Acceleration (x,y,z): accel1, accel2, accel3, Fl, Fr, Bl, Br
    imuData->accelVcu.x = 0.546f;
    imuData->accelVcu. y= 0.546f;
    imuData->accelVcu.z = 0.546f;
    imuData->accelHvc.x = 0.546f;
    imuData->accelHvc.y = 0.546f;
    imuData->accelHvc.z = 0.546f;
    imuData->accelPdu.x = 0.546f;
    imuData->accelPdu.y = 0.546f;
    imuData->accelPdu.z = 0.546f;
    imuData->accelFl.x = 0.546f;
    imuData->accelFl.y = 0.546f;
    imuData->accelFl.z = 0.546f;
    imuData->accelFr.x = 0.546f;
    imuData->accelFr.y = 0.546f;
    imuData->accelFr.z = 0.546f;
    imuData->accelBl.x = 0.546f;
    imuData->accelBl.y = 0.546f;
    imuData->accelBl.z = 0.546f;
    imuData->accelBr.x = 0.546f;
    imuData->accelBr.y = 0.546f;
    imuData->accelBr.z = 0.546f;


    // IMU Gyro (x,y,z): gyro1, gyro2, gyro3
    imuData->gyroVcu.x = 34;
    imuData->gyroVcu.y = 34;
    imuData->gyroVcu.z = 34;
    imuData->gyroHvc.x = 34;
    imuData->gyroHvc.y = 34;
    imuData->gyroHvc.z = 34;
    imuData->gyroPdu.x = 34;
    imuData->gyroPdu.y = 34;
    imuData->gyroPdu.z = 34;


    // Wheel Speed: fl, fr, bl, br
    wheelDisplacements->fl = 4.5f;
    wheelDisplacements->fr = 4.5f;
    wheelDisplacements->bl = 4.5f;
    wheelDisplacements->br = 4.5f;

    // inverter Voltage
    inverterStatus->voltage = 0.39f;

    // inverter current
    inverterStatus->current = 0.39f;


    // inverter RPM
    inverterStatus->rpm = 67;

    // inverter Torque
    inverterStatus->torqueActual = 9.3f;
}

static void cellular_sendTelemetryLow(VcuOutput *vcuCoreOutput, HvcStatus *hvcStatus,
                                       PduStatus *pduStatus, InverterStatus *inverterStatus,
                                       AnalogVoltages *analogVoltages, WheelDisplacements *wheelDisplacements,
                                       ImuData *imuData, GpsData *gpsData)
{
    std::string dataToEncode = "";
    uint8_t arr[406];
    uint8_t *ptr = arr;
    // Where do I get this data?

    std::string gps_stuff;


    uint16_t year = 2024;
    cellular_split16(ptr, year);

    uint8_t month = 2;
    cellular_split8(ptr, month);

    uint8_t day = 2;
    cellular_split8(ptr, day);

    uint8_t hour = 2;
    cellular_split8(ptr, hour);

    uint8_t minute = 2;
    cellular_split8(ptr, minute);

    uint16_t seconds = (uint16_t)(2 / 0.001f);
    cellular_split16(ptr, seconds);

    // Where do I get this?
    uint32_t currentErrors = 1;
    cellular_split32(ptr, currentErrors);

    uint32_t latchingFaults = 1;
    cellular_split32(ptr, latchingFaults);

    uint16_t cellVoltages[140];
    uint8_t cellTemperature[90];

    for(int i = 0; i < 140; i++)
    {
        cellular_split16(ptr, cellVoltages[i]);
    }

    for(int i = 0; i < 90; i++)
    {
        cellular_split8(ptr, cellTemperature[i]);
    }

    uint16_t hvSOC = (uint16_t)(hvcStatus->stateOfCharge / 0.01f);
    cellular_split16(ptr, hvSOC);

    uint16_t lvSOC = (uint16_t)(pduStatus->lvSoC / 0.01f);
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



    char encoded[532];
    cellular_Base64encode(encoded, arr, 406);
    for(int i = 0; i < 532; i++)
    {
        dataToEncode = dataToEncode + encoded[i];
    }
    std::string command = "AT+UMQTTC=2,0,0,\"/l\",\"" + dataToEncode + "\"\r";
    std::string response = "\r\r\n+UMQTTC: 2,1\r\r\n\r\nOK\r\n";
    cellular_sendNonBlocking(command);
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
  volatile int y = 10;
  command = "AT+UMQTT=0,\"Car\"\r";
  response = "\r\r\n+UMQTT: 0,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receiveAny(64, response,  1000);
  if (response.size() > 23)
  {
      return;
  }
  y++;
  command = "AT+UMQTT=1,1883\r";
  response = "\r\r\n+UMQTT: 1,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true, 1000);
    y++;
  command = "AT+UMQTT=2,\"ec2-52-14-184-219.us-east-2.compute.amazonaws.com\",1883\r";
  response = "\r\r\n+UMQTT: 2,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true, 1000);
    y++;
  command = "AT+UMQTT=10,3600\r";
  response = "\r\r\n+UMQTT: 10,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true, 1000);
    y++;
  command = "AT+UMQTT=12,1\r";
  response = "\r\r\n+UMQTT: 12,1\r\r\n\r\nOK\r\n";
  cellular_send(&command);
  cellular_receive(response, true, 1000);
    y++;
  command = "AT+UMQTTC=1\r";
  response = "\r\r\n+UMQTTC: 1,1\r\r\n\r\nOK\r\n\r\r\n+UUMQTTC: 1,0\r\r\n";
  cellular_send(&command);
  cellular_receive(response, true, 5000);
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


static void cellular_registerTMobile()
{
  volatile int y = 10;
  std::string command = "AT+COPS?\r";
  std::string response;
  cellular_send(&command);
  cellular_receiveAny(500, response, 5000);
  y++;
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
  y++;

}

static void cellular_subscribe()
{
    std::string command = "AT+UMQTTC=4,0,\"#\"\r";

    volatile int y = 0;
    cellular_send(&command);
    uint8_t buffer[256];
    memset(buffer, 0, 256);
    HAL_UART_Receive(&huart7, (uint8_t *) buffer, 256, 5000);
//    cellular_receiveAny(500, response, 10000);
    y++;

}


static void cellular_poll()
{
    volatile int x = 99;
    std::string command = "AT+UMQTTC=6\r";
    std::string response;
    cellular_send(&command);
    uint8_t buffer[256];
    memset(buffer, 0, 256);
    HAL_UART_Receive(&huart7, (uint8_t *) buffer, 256, 500);
    std::string num_response = "";
    x++;
    for(int i = 0; i < 256; i++)
    {

        uint8_t current = buffer[i];
        if (current == uint8_t ('h'))
        {
            x++;
            x*=x;
        }
    }
}

extern VcuOutput vcuCoreOutput;
extern GpsData gpsData;

static void cellular_respondToText(std::string* senderPhoneNumber, std::string* message) {
  std::string latString;
  if(gpsData.latitude > 0) {
    latString = std::to_string(gpsData.latitude);
    latString += " N";
  } else {
    latString = std::to_string(-gpsData.latitude);
    latString += " S";
  }

  std::string longString;
  if(gpsData.longitude > 0) {
    longString = std::to_string(gpsData.longitude);
    longString += " E";
  } else {
    longString = std::to_string(-gpsData.longitude);
    longString += " W";
  }

  std::string response = "I'm parked at " + latString + ", " + longString + "." ;
  if(vcuCoreOutput.prndlState) {
    response = "I'm driving right now. Talk to you later.";
  }
  cellular_sendText(senderPhoneNumber, &response);
}

// public methods
uint8_t rxbuffer[1024];

void cellular_init()
{
    volatile int x = 99;

    cellular_disableEcho();
    cellular_testConnection();
    cellular_disableEcho();
    cellular_disableEcho();
    cellular_disableEcho();
    cellular_testConnection();
    x++;
    cellular_registerTMobile();
    x++;
    cellular_mqttInit();
    x++;


//    VcuOutput vcuCoreOutput;
//    HvcStatus hvcStatus;
//    PduStatus pduStatus;
//    InverterStatus inverterStatus;
//    AnalogVoltages analogVoltages;
//    WheelDisplacements wheelDisplacements;
//    ImuData imuData;
//    GpsData gpsData;
    x++;
    uint32_t error = HAL_UARTEx_ReceiveToIdle_DMA(&huart7, (uint8_t *) cell_tempLine, MAX_CELL_LINE_SIZE);
    x += error;
//    cellular_createDummyHFSend(&vcuCoreOutput, &hvcStatus, &pduStatus, &inverterStatus, &analogVoltages, &wheelDisplacements, &imuData, &gpsData);
//    x++;
//    cellular_sendTelemetryHigh(&vcuCoreOutput, &hvcStatus, &pduStatus, &inverterStatus, &analogVoltages, &wheelDisplacements, &imuData, &gpsData);

//    x++;
//    cellular_subscribe();
//    x++;
//    while(1)
//    {
//        cellular_poll();
//        x++;
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


void cellular_sendText(std::string* phoneNumber, std::string* message) {
  std::string command;
  std::string response;

  command = "AT+CMGF=1\r";
  cellular_sendAndExpectOk(&command);

  command = "AT+CMGS=\"" + *phoneNumber + "\"\r";
  response = "\r\n> ";
  cellular_send(&command);
  cellular_receive(response, true, 5000);

  cellular_send(message);

  command = "\x1A"; // Ctrl+Z character to indicate end of message
  cellular_send(&command);

  cellular_receiveAny(64, response, 30000);
  uint8_t index = response.size() - 6;
  const char* okBegin = response.c_str() + index;
  if(strcmp(okBegin, "\r\nOK\r\n") != 0) {
    Error_Handler();
  }
}

void cellular_respondToTexts() {
  std::string command;
  std::string response;
  std::string sender;
  std::string message;

    HAL_UART_DMAPause(&huart7);
  command = "AT+CMGF=1\r";
  cellular_sendAndExpectOk(&command);

  command = "AT+CMGL\r";
  cellular_send(&command);
  cellular_receiveAny(2048, response, 1000);

  // delete read messages to save space
  command = "AT+CMGD=0,3\r";
  cellular_sendAndExpectOk(&command);

  for(int i = 0; i < response.size();) {
    i += 2;
    if(i >= response.size() || (response.at(i) == 'O' && response.at(i+1) == 'K')) {
      break;
    }
    if(response.at(i) != '+') {
      break;
    }
    int commas = 0;
    for(; i < response.size(); i++) {
      if(response.at(i) == ',') {
        commas++;
        if(commas == 2) {
          break;
        }
      }
    }
    i += 2;
    if(i >= response.size()) {
      break;
    }
    sender = "";
    for(; i < response.size(); i++) {
      if(response.at(i) == '\"') {
        break;
      }
      sender += response.at(i);
    }
    if(i >= response.size()) {
      break;
    }
    for(; i < response.size(); i++) {
      if(response.at(i) == '\n') {
        break;
      }
    }
    i += 1;
    if(i >= response.size()) {
      break;
    }
    message = "";
    for(; i < response.size(); i++) {
      if(response.at(i) == '\r') {
        break;
      }
      message += response.at(i);
    }
    if(i >= response.size()) {
      break;
    }

    cellular_respondToText(&sender, &message);
      HAL_UART_DMAResume(&huart7);
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
    if(!startTimeSent)
    {
        cellular_sendStartTime(gpsData);

    }
    else
    {
        static float lastTime = 0;
        float nowTime = clock_getTime();
        if(nowTime - lastTime > 0.05f) {
            lastTime = nowTime;
            cellular_sendTelemetryHigh(vcuCoreOutput, hvcStatus,
                                       pduStatus, inverterStatus,
                                       analogVoltages, wheelDisplacements,
                                       imuData, gpsData);
        }

    }




  // check and respond to text messages once per second
  static float lastTextTime = 0;
  float nowTextTime = clock_getTime();
  if(nowTextTime - lastTextTime > 1.0f) {
      lastTextTime = nowTextTime;
    cellular_respondToTexts();
  }
}
