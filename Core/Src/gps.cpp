#include "gps.h"
#include <cstring>

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

    HAL_UART_Receive_DMA(&hlpuart, (uint8_t *) gps_rx_buff, 1);

    // Baud rate is hard-coded to 115200 bps
    // Note: we may have to connect an arduino to the GPS module to change the baud rate
    auto status = static_cast<HAL_StatusTypeDef>(send_command(PMTK_SET_BAUD_115200));
    if(status != HAL_OK) {
        return;
    }
    //sends both GGA and RMC data
    status = static_cast<HAL_StatusTypeDef>(send_command(PMTK_SET_NMEA_OUTPUT_RMCGGA));
    if(status != HAL_OK) {
        return;
    }
    //sets the update rate to 1 Hz
    status = static_cast<HAL_StatusTypeDef>(send_command(PMTK_SET_NMEA_UPDATE_1HZ));
    if(status != HAL_OK) {
        return;
    }
    //requests the antenna status
    status = static_cast<HAL_StatusTypeDef>(send_command(PGCMD_ANTENNA));
    if(status != HAL_OK) {
        return;
    }
    is_ready = true;
}

int Adafruit_GPS::send_command(const char *cmd) {
    std::string cmd_str = cmd;
    if((cmd_str.substr(0, 5) != "$PMTK") && (cmd_str.substr(0, 4) != "$PGC")) {
        return 1;
    }
    return HAL_UART_Transmit(&uart_handler, (uint8_t *) cmd, strlen(cmd), HAL_MAX_DELAY);
}

char Adafruit_GPS::read_command(){
    if(!gps_eof || paused){
        return 0;
    }
    strncpy(last_line, (char *) gps_curr_line, 128);
    gps_eof = false;
    received = true;
    gps_line_ofs = 0;
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
        HAL_UART_Receive_DMA(&uart_handler, (uint8_t *) gps_rx_buff, 1);
    }
    return paused;
}

char* Adafruit_GPS::lastNMEA() {
    received = false;
    return last_line;
}
