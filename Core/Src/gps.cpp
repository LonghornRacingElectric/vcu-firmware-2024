#include "gps.h"
#include "faults.h"
#include <cstring>

Adafruit_GPS gps(hlpuart1);

Adafruit_GPS::Adafruit_GPS(UART_HandleTypeDef &hlpuart) : uart_handler(hlpuart) {
    this->uart_handler = hlpuart;

    paused = false;
    received = false;
    hour = minute = seconds = year = month = day = fixquality = fixquality_3d = satellites = 0;
    latitude = longitude = geoidheight = altitude = HDOP = VDOP = PDOP = speed = angle = 0.0;
    latitude_dir = longitude_dir = 0;
    antenna = UNKNOWN;

    has_fix = false;
    is_ready = false;
    milliseconds = 0;

    latitudeDegrees = longitudeDegrees = 0.0;
    latitude_fixed = longitude_fixed = 0;

    last_line.reserve(MAX_GPS_LINE_SIZE);
}

int Adafruit_GPS::send_command(const char *cmd) {
    std::string cmd_str = cmd;
    if((cmd_str.substr(0, 5) != "$PMTK") && (cmd_str.substr(0, 4) != "$PGC")) {
        return 1;
    }
    return HAL_UART_Transmit(&uart_handler, (uint8_t *) cmd, strlen(cmd), HAL_MAX_DELAY);
}

char Adafruit_GPS::read_command(){
    if(!gps_completeLine || paused){
        return 0;
    }
    last_line = gps_currLine;
    gps_completeLine = false;
    received = true;
    return last_line[0];
}

bool Adafruit_GPS::newNMEAreceived() const {
    return received;
}

bool Adafruit_GPS::pause(bool p) {
    paused = p;
    if(paused){
        HAL_UART_DMAPause(&uart_handler);
    } else {
        waitForNewMessage();
    }
    return paused;
}

string Adafruit_GPS::lastNMEA() {
    received = false;
    return last_line;
}

int Adafruit_GPS::waitForNewMessage() {
  auto error = HAL_UARTEx_ReceiveToIdle_DMA(&uart_handler, (uint8_t *) gps_currLine, MAX_GPS_LINE_SIZE);
  if (error != HAL_OK) {
    FAULT_SET(&vcu_fault_vector, FAULT_VCU_GPS);
  }
  return error;
}

void gps_init() {
    gps.waitForNewMessage();
    // Baud rate is hard-coded to 115200 bps
    // Note: we may have to connect an arduino to the GPS module to change the baud rate
    auto status = static_cast<HAL_StatusTypeDef>(gps.send_command(PMTK_SET_BAUD_115200));
    if(status != HAL_OK) {
        return;
    }
    //sends both GGA and RMC data
    status = static_cast<HAL_StatusTypeDef>(gps.send_command(PMTK_SET_NMEA_OUTPUT_RMCGGA));
    if(status != HAL_OK) {
        return;
    }
    //sets the update rate to 1 Hz
    status = static_cast<HAL_StatusTypeDef>(gps.send_command(PMTK_SET_NMEA_UPDATE_10HZ));
    if(status != HAL_OK) {
        return;
    }
    //requests the antenna status
    status = static_cast<HAL_StatusTypeDef>(gps.send_command(PGCMD_ANTENNA));
    if(status != HAL_OK) {
        return;
    }
    gps.is_ready = true;
}

void gps_periodic(GpsData* gpsData) {
    gps.read_command();
    if(gps.newNMEAreceived()){
        if(!gps.parse(gps.lastNMEA())){
            return;
        }
        if(gps.has_fix){
            gpsData->latitude = gps.latitudeDegrees;
            gpsData->longitude = gps.longitudeDegrees;
            gpsData->speed = gps.speed;
            gpsData->heading = gps.angle;
            gpsData->hour = gps.hour;
            gpsData->minute = gps.minute;
            gpsData->seconds = gps.seconds;
            gpsData->year = gps.year;
            gpsData->month = gps.month;
            gpsData->day = gps.day;
            gpsData->timeMillis = HAL_GetTick() - gps.milliseconds;
            gps.milliseconds = HAL_GetTick();
        }
    }
}
