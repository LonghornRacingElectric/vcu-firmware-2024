#include "bevo.h"

static CanOutbox gpsFrame1Outbox;
static CanOutbox gpsFrame2Outbox;
static CanOutbox gpsFrame3Outbox;

void bevo_init()
{
    can_addOutbox(GPS_FRAME_1, GPS_PERIOD, &gpsFrame1Outbox);
    can_addOutbox(GPS_FRAME_2, GPS_PERIOD, &gpsFrame2Outbox);
    can_addOutbox(GPS_FRAME_3, GPS_PERIOD, &gpsFrame3Outbox);

    gpsFrame1Outbox.dlc = 8;
    gpsFrame2Outbox.dlc = 8;
    gpsFrame3Outbox.dlc = 6;
}

void bevo_send(GpsData *g)
{
    // Frame 1: lat, long, speed, heading
    can_writeFloat(int16_t, &gpsFrame1Outbox, 0, g->latitude,  0.001f);
    can_writeFloat(int16_t, &gpsFrame1Outbox, 2, g->longitude, 0.001f);
    can_writeFloat(int16_t, &gpsFrame1Outbox, 4, g->speed,     0.001f);
    can_writeFloat(int16_t, &gpsFrame1Outbox, 6, g->heading,   0.001f);

    // Frame 2: hour, minute, seconds, year
    can_writeFloat(int16_t, &gpsFrame2Outbox, 0, g->hour,    1.0f);
    can_writeFloat(int16_t, &gpsFrame2Outbox, 2, g->minute,  1.0f);
    can_writeFloat(int16_t, &gpsFrame2Outbox, 4, g->seconds, 1.0f);
    can_writeFloat(int16_t, &gpsFrame2Outbox, 6, g->year,    1.0f);

    // Frame 3: month, day, millis
    can_writeFloat(int16_t, &gpsFrame3Outbox, 0, g->month,  1.0f);
    can_writeFloat(int16_t, &gpsFrame3Outbox, 2, g->day,    1.0f);
    can_writeFloat(int16_t, &gpsFrame3Outbox, 4, g->millis, 1.0f);
}
