//
// Created by Matthew on 12/19/2023.
//

#ifndef VCU_FIRMWARE_2024_SDSPI_H
#define VCU_FIRMWARE_2024_SDSPI_H

#include <cstdint>

void sdspi_init();
void sdspi_readBlock(uint32_t address, uint8_t data[512]);

bool sdspi_isCardPresent();

#endif //VCU_FIRMWARE_2024_SDSPI_H
