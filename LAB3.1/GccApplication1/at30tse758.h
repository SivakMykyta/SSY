// at30tse758.h
#ifndef AT30TSE758_H
#define AT30TSE758_H

#include <stdint.h>

#define TempSensorAddrR 0b10010111
#define TempSensorAddrW 0b10010110

// Funkce pro nastavení rozlišení
uint8_t at30_set_precision(uint8_t precision);

// Funkce pro čtení teploty
float at30_read_temp(void);

#endif

// at30tse758.c
#include "at30tse758.h"
#include "i2c.h"

#define Temp_configRegister 0x01
#define Temp_tempRegister 0x00

uint8_t at30_set_precision(uint8_t precision) {
    uint16_t config_register = 0;
    config_register |= (uint16_t)(precision << 0);  // Nastavení rozlišení

    i2c_start();
    i2c_write(TempSensorAddrW);
    if (i2c_get_status() != 0x18) {
        return 0;  // Chyba při zápisu adresy
    }
    i2c_write(Temp_configRegister);
    if (i2c_get_status() != 0x28) {
        return 0;  // Chyba při zápisu do konfiguračního registru
    }
    i2c_write((uint8_t)(config_register >> 8));
    if (i2c_get_status() != 0x28) {
        return 0;  // Chyba při zápisu dat
    }
    i2c_write((uint8_t)(config_register));
    if (i2c_get_status() != 0x28) {
        return 0;  // Chyba při zápisu dat
    }
    i2c_stop();
    return 1;
}

float at30_read_temp(void) {
    uint8_t buffer[2];
    int16_t temperature;

    // Nastavit pointer na registr teploty
    i2c_start();
    i2c_write(TempSensorAddrW);
    if (i2c_get_status() != 0x18) {
        return -1;  // Chyba při zápisu adresy
    }
    i2c_write(Temp_tempRegister);
    if (i2c_get_status() != 0x28) {
        return -1;  // Chyba při zápisu do registru
    }
    i2c_stop();

    // Čtení teploty
    i2c_start();
    i2c_write(TempSensorAddrR);
    if (i2c_get_status() != 0x40) {
        return -1;  // Chyba při čtení adresy
    }

    buffer[0] = i2c_read_ack();
    if (i2c_get_status() != 0x50) {
        return -1;  // Chyba při čtení dat
    }
    buffer[1] = i2c_read_nack();
    if (i2c_get_status() != 0x58) {
        return -1;  // Chyba při čtení dat
    }
    i2c_stop();

    temperature = (buffer[0] << 8) | buffer[1];
    return (float)temperature / 256.0;  // Převod na teplotu
}
