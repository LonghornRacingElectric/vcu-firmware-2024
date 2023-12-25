//
// Created by Matthew on 12/19/2023.
//

#ifndef VCU_FIRMWARE_2024_SDSPI_H
#define VCU_FIRMWARE_2024_SDSPI_H

#include <cstdint>

void sdspi_init();

void sdspi_deselect();
void sdspi_select();

bool sdspi_isCardPresent();

void sdspi_start();

void sdspi_send(uint8_t* data, uint32_t length);
void sdspi_receive(uint8_t* data, uint32_t length);

#endif //VCU_FIRMWARE_2024_SDSPI_H
