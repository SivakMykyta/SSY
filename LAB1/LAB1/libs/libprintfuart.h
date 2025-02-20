/*
 * UART.h
 *
 * Created: 12.2.2018 13:49:42
 *  Author: Krajsa
 */ 
#include <stdio.h>
#ifndef UART_H_
#define UART_H_

void UART_init(uint16_t Baudrate);
void UART_SendChar(uint8_t data);
uint8_t UART_GetChar( void );
void UART_SendString(char *text);
int printCHAR(char character, FILE *stream);
#endif /* UART_H_ */