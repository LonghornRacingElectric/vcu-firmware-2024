#ifndef VCU_FIRMWARE_2024_GPS_H
#define VCU_FIRMWARE_2024_GPS_H

#include <stdint.h>

typedef struct GpsData {
  float latitude;
  float longitude;
  float speed;
  float heading;
  uint64_t timeMillis;
} GpsData;

void gps_init();

void gps_periodic(GpsData* gpsData);

#endif //VCU_FIRMWARE_2024_GPS_H
