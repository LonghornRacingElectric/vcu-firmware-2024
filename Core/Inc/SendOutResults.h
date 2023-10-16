//
// Created by yashk on 10/16/2023.
//

#ifndef VCU_FIRMWARE_2024_SENDCELLOUTPUT_H
#define VCU_FIRMWARE_2024_SENDCELLOUTPUT_H

#include "firmware_faults.h"
#include "commsvars.h"
#include "VcuModel.h"
#include "library.h"

int Send_Out_Results(VcuInput* , VcuParameters* , VcuOutput* , BSPD* , uint32_t);


#endif //VCU_FIRMWARE_2024_SENDCELLOUTPUT_H
