#ifndef VCU_FIRMWARE_2024_GPS_H
#define VCU_FIRMWARE_2024_GPS_H

// TODO Adafruit GPS

typedef struct GpsData {
  float gpsLat;
  float gpsLong;
  float gpsSpeed;
  float gpsHeading;
} GpsData;

void gps_init();

void gps_periodic(GpsData* gpsData);

#endif //VCU_FIRMWARE_2024_GPS_H
