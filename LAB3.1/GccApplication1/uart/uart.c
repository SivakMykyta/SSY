/*
 * uart.c
 *
 * Created: 17.02.2022 13:41:57
 *  Author: Ondra
 */ 

#include "../makra.h"
#include <stdio.h>

void UART_init(uint16_t Baudrate){
		int ubrr=((F_CPU/16/Baudrate)-1);
		UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);// Async, Parity-Disabled, 1 Stop bit, 8 data bits
		//sbi(UCSR1C,UCSZ11);
		//sbi(UCSR1C,UCSZ10);
		UBRR1H = (uint8_t)(ubrr>>8);
		UBRR1L = (uint8_t)ubrr;
		UCSR1B = (1<<RXEN1)|(1<<TXEN1);// Enable RX/TX
}

void UART_SendChar(uint8_t data)
{
	while ( !( UCSR1A & (1<<UDRE0)) )
	;
	UDR1 = data;
}
uint8_t UART_GetChar( void )
{
	while ( !(UCSR1A & (1<<RXC0)) )
	;
	return UDR1;
}


void UART_SendString(char *text)
{
	while (*text != 0x00)
	{
		UART_SendChar(*text);
		text++;
	}
}


int printCHAR(char character, FILE *stream)
{
	while ((UCSR1A & (1 << UDRE1)) == 0) {};

	UDR1 = character;

	return 0;
}