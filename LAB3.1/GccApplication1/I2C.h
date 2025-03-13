// i2c.h
#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
uint8_t i2c_get_status(void);

#endif

// i2c.c
#include "i2c.h"
#include <avr/io.h>
#include <util/delay.h>

void i2c_init(void) {
    TWSR = 0x00;   // Nastavit TWI prescaler na 1
    TWBR = 0x02;   // Nastavit TWI bitrate na 400 kHz
    TWCR = (1<<TWEN);  // Povolit TWI
}

void i2c_start(void) {
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);  // Start podmínka
    while ((TWCR & (1<<TWINT)) == 0);  // Čekat na dokončení
}

void i2c_stop(void) {
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);  // Stop podmínka
}

void i2c_write(uint8_t data) {
    TWDR = data;  // Uložit data do TWI datového registru
    TWCR = (1<<TWINT) | (1<<TWEN);  // Odeslat data
    while ((TWCR & (1<<TWINT)) == 0);  // Čekat na dokončení
}

uint8_t i2c_read_ack(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);  // Povolit ACK
    while ((TWCR & (1<<TWINT)) == 0);  // Čekat na dokončení
    return TWDR;  // Vrátit přijatá data
}

uint8_t i2c_read_nack(void) {
    TWCR = (1<<TWINT) | (1<<TWEN);  // Povolit NACK
    while ((TWCR & (1<<TWINT)) == 0);  // Čekat na dokončení
    return TWDR;  // Vrátit přijatá data
}

uint8_t i2c_get_status(void) {
    uint8_t status;
    status = TWSR & 0xF8;  // Získat stavový kód
    return status;
}
