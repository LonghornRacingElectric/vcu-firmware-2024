//
// Created by alice on 11/9/2025.
//

#include "bevo.h"

static CanOutbox latitudeOutbox;
static CanOutbox longitudeOutbox;
static CanOutbox speedOutbox;
static CanOutbox headingOutbox;
static CanOutbox hourOutbox;
static CanOutbox minuteOutbox;
static CanOutbox secondsOutbox;
static CanOutbox yearOutbox;
static CanOutbox monthOutbox;
static CanOutbox dayOutbox;
static CanOutbox millisOutbox;

void bevo_init()
{
    can_addOutbox(GPS_LATITUDE, GPS_PERIOD, &latitudeOutbox);
    can_addOutbox(GPS_LONGITUDE, GPS_PERIOD, &longitudeOutbox);
    can_addOutbox(GPS_SPEED, GPS_PERIOD, &speedOutbox);
    can_addOutbox(GPS_HEADING, GPS_PERIOD, &headingOutbox);
    can_addOutbox(GPS_HOUR, GPS_PERIOD, &hourOutbox);
    can_addOutbox(GPS_MINUTE, GPS_PERIOD, &minuteOutbox);
    can_addOutbox(GPS_SECONDS, GPS_PERIOD, &secondsOutbox);
    can_addOutbox(GPS_YEAR, GPS_PERIOD, &yearOutbox);
    can_addOutbox(GPS_MONTH, GPS_PERIOD, &monthOutbox);
    can_addOutbox(GPS_DAY, GPS_PERIOD, &dayOutbox);
    can_addOutbox(GPS_MILLIS, GPS_PERIOD, &millisOutbox);

}
void bevo_send(GpsData *gpsData)
{
    can_writeFloat(int16_t, &latitudeOutbox, 0, gpsData->latitude, 0.001f);
    can_writeFloat(int16_t, &longitudeOutbox, 0, gpsData->longitude, 0.001f);
    can_writeFloat(int16_t, &speedOutbox, 0, gpsData->speed, 0.001f);
    can_writeFloat(int16_t, &headingOutbox, 0, gpsData->heading, 0.001f);
    can_writeFloat(int16_t, &hourOutbox, 0, gpsData->hour, 1.0f);
    can_writeFloat(int16_t, &minuteOutbox, 0, gpsData->minute, 1.0f);
    can_writeFloat(int16_t, &secondsOutbox, 0, gpsData->seconds, 1.0f);
    can_writeFloat(int16_t, &yearOutbox, 0, gpsData->year, 1.0f);
    can_writeFloat(int16_t, &monthOutbox, 0, gpsData->month, 1.0f);
    can_writeFloat(int16_t, &dayOutbox, 0, gpsData->day, 1.0f);
    can_writeFloat(int16_t, &millisOutbox, 0, gpsData->millis, 1.0f);

}
