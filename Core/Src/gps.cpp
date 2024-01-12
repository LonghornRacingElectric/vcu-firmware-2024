#include "gps.h"
#include <cstring>

uint8_t curr_line[128] = {0};
char rx_buff[1] = {0};
volatile uint8_t line_ofs = 0;
bool eof = false;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART3) {
        curr_line[line_ofs++] = rx_buff[0];
        if(curr_line[line_ofs - 1] == '\n' || curr_line[line_ofs - 1] == '\r') {
            eof = true;
        }
        if(line_ofs >= 128) {
            eof = true;
            line_ofs = 0;
        }
        HAL_UART_Receive_DMA(huart, (uint8_t *) rx_buff, 1);
    }
}

Adafruit_GPS::Adafruit_GPS(UART_HandleTypeDef &hlpuart) : hlpuart(hlpuart) {
    this->hlpuart = hlpuart;

    paused = false;
    line_ofs = 0;
    hour = minute = seconds = year = month = day = fixquality = fixquality_3d = satellites = antenna = 0;
    latitude = longitude = geoidheight = altitude = magvariation = HDOP = VDOP = PDOP = speed = angle = 0.0;
    latitude_dir = longitude_dir = magnetic_dir = 0;

    has_fix = false;
    is_ready = false;
    milliseconds = 0;

    latitudeDegrees = longitudeDegrees = 0.0;
    latitude_fixed = longitude_fixed = 0;

    HAL_UART_Receive_DMA(&hlpuart, (uint8_t *) rx_buff, 1);

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
    return HAL_UART_Transmit(&hlpuart, (uint8_t *) cmd, strlen(cmd), HAL_MAX_DELAY);
}

void Adafruit_GPS::read_command(){
    if(!eof){
        return;
    }
    strncpy(last_line, (char *) curr_line, 128);
    eof = false;
    received = true;
    line_ofs = 0;
}

bool Adafruit_GPS::newNMEAreceived() const {
    return received;
}

bool Adafruit_GPS::pause(bool p) {
    paused = p;
    return paused;
}

char* Adafruit_GPS::lastNMEA() {
    received = false;
    return last_line;
}
