#ifndef VCU_FIRMWARE_2024_GPS_TIME_H
#define VCU_FIRMWARE_2024_GPS_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GpsFatTimestamp {
  uint8_t hour;
  uint8_t minute;
  uint8_t seconds;
  uint8_t year;
  uint8_t month;
  uint8_t day;
} GpsFatTimestamp;

extern volatile GpsFatTimestamp gpsFatTimestamp;

#ifdef __cplusplus
}
#endif

#endif // VCU_FIRMWARE_2024_GPS_TIME_H
