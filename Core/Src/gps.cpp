#include "gps.h"
#include <cstring>

int count = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == LPUART1) {
        count++;
        received = true;
    }
}

Adafruit_GPS::Adafruit_GPS(UART_HandleTypeDef &hlpuart) : hlpuart(hlpuart) {
    this->hlpuart = hlpuart;

    received = false;
    paused = false;
    line_ofs = 0;
    hour = minute = seconds = year = month = day = fixquality = fixquality_3d = satellites = antenna = 0;
    latitude = longitude = geoidheight = altitude = magvariation = HDOP = VDOP = PDOP = speed = angle = 0.0;
    latitude_dir = longitude_dir = magnetic_dir = 0;

    has_fix = false;
    milliseconds = 0;

    latitudeDegrees = longitudeDegrees = 0.0;
    latitude_fixed = longitude_fixed = 0;

    // Baud rate is hard-coded to 115200 bps
    // Note: we may have to connect an arduino to the GPS module to change the baud rate
    send_command(PMTK_SET_BAUD_115200);
    //sends both GGA and RMC data
    send_command(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    //sets the update rate to 1 Hz
    send_command(PMTK_SET_NMEA_UPDATE_1HZ);
    //requests the antenna status
    send_command(PGCMD_ANTENNA);
}

int Adafruit_GPS::send_command(const char *cmd) {
    std::string cmd_str = cmd;
    if(cmd_str.substr(0, 6) != "$PMTK") {
        return 1;
    }
    return HAL_UART_Transmit(&hlpuart, (uint8_t *) cmd, strlen(cmd), 100);
}

void Adafruit_GPS::read_command(){
    if(!received) {
        return;
    }
    HAL_UART_Receive_DMA(&hlpuart, (uint8_t *) &curr_line[line_ofs], count);
    line_ofs += count;
    count = 0;
}

bool Adafruit_GPS::newNMEAreceived() const {
    return received;
}

bool Adafruit_GPS::pause(bool p) {
    paused = p;
    return paused;
}

std::string Adafruit_GPS::lastNMEA() {
    received = false;
    return last_line;
}
