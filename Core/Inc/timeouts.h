#ifndef VCU_FIRMWARE_2024_TIMEOUTS_H
#define VCU_FIRMWARE_2024_TIMEOUTS_H

#include "faults.h"

#define timeout_fault(boolean, fault)  \
    do { \
        if (boolean) { \
            FAULT_SET(&vcu_fault_vector, fault); \
        } else { \
            FAULT_CLEAR(&vcu_fault_vector, fault); \
        } \
    } while(0)

// I defined timouts to be 10 times the period of expected reception rate

#define INV_TIMEOUT_SLOW 1.0f
#define INV_TIMEOUT_FAST 0.1f
#define INV_TIMEOUT_VERYFAST 0.03f

#define PDU_TIMEOUT_SLOW 1.0f
#define PDU_TIMEOUT_FAST 0.03f

#define HVC_TIMEOUT_SLOW 1.0f
#define HVC_TIMEOUT_FAST 0.03f
#define HVC_TIMEOUT_TELEM 5.0f

#define UNS_TIMEOUT 0.03f

#endif //VCU_FIRMWARE_2024_TIMEOUTS_H
