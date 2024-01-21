//
// Created by Matthew on 12/19/2023.
//

#ifndef VCU_FIRMWARE_2024_BBSPI_H
#define VCU_FIRMWARE_2024_BBSPI_H

#include <cstdint>

void bbspi_init();

void bbspi_deselect();
void bbspi_selectImu();
void bbspi_selectEeprom();

void bbspi_send(uint8_t* data, uint32_t length);
void bbspi_receive(uint8_t* data, uint32_t length);

#endif //VCU_FIRMWARE_2024_BBSPI_H
