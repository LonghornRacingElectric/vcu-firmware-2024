//
// Created by alice on 11/9/2025.
//

#ifndef BEVO_H
#define BEVO_H

#include <cstdint>
#include "VcuModel.h"
#include "angel_can.h"
#include "angel_can_ids.h"
#include "gps.h"

#define GPS_PERIOD 0.1f

void bevo_init();
void bevo_send(GpsData* gps_data);
#endif //BEVO_H
