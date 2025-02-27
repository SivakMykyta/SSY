/*
 * GccApplication1.c
 *
 * Created: 2/27/2025 13:53:48
 * Author : Student
 */ 
/************************************************************************/
/* INCLUDE                                                              */
/************************************************************************/

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include "./makra.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define BAUD 38400
#define MYUBRR ((F_CPU/(16*BAUD))-1)

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

uint8_t textt = 0x41;
char *string = "Hello";

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void board_init();
void USART_init(uint16_t _ubrr);
void USART_Transmit(uint8_t data);
uint8_t USART_GetChar(void);
void UART_SendChar(uint8_t data);
void UART_SendString(char *text);
void showMenu();
void handleMenuOption(uint8_t option);

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

void USART_init(uint16_t _ubrr) {
	UBRR1H = (uint8_t)(_ubrr>>8);
	UBRR1L = (uint8_t)(_ubrr);
	
	UCSR1B |= (1<<RXEN1) | (1<<TXEN1);
	
	UCSR1C |= (1<<UCSZ11) | (1<<UCSZ10);
}

void USART_Transmit(uint8_t data) {
	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}

uint8_t USART_GetChar(void) {
	while (!(UCSR1A & (1<<RXC1)));
	return UDR1;
}

void UART_SendChar(uint8_t data) {
	while (!(UCSR1A & (1<<UDRE1)));
	UDR1 = data;
}

void UART_SendString(char *text) {
	while (*text != 0x00) {
		UART_SendChar(*text);
		text++;
	}
}

void board_init() {
	USART_init(MYUBRR);
}

void showMenu() {
    UART_SendString("\033[2J\033[H");  // Clear screen and move cursor to home position
    UART_SendString("\nSelect an option:\r\n");
    UART_SendString("1. Show A (Pismeno)\r\n");
    UART_SendString("2. Show ABCDEFG\r\n");
    UART_SendString("3. Option 3 (Blank)\r\n");
    UART_SendString("4. Option 4 (Blank)\r\n");
    UART_SendString("5. Refresh\n");
}



void handleMenuOption(uint8_t option) {
    switch (option) {
        case '1':
            UART_SendString("You selected Option 1: Show A (Pismeno)\n");
            UART_SendChar(textt);
            break;
        case '2':
            UART_SendString("You selected Option 2: Show ABCDEFG\n");
            UART_SendString("ABCDEFG\n");
            break;
        case '3':
            UART_SendString("You selected Option 3: (Blank)\n");
            break;
        case '4':
            UART_SendString("You selected Option 4: (Blank)\n");
            break;
        case '5':
            UART_SendString("Refreshing the menu...\n");
            break;
        default:
            UART_SendString("Invalid option, try again.\n");
            break;
    }
}

int main(void) {
    board_init();
    
    while(1) {
        showMenu();
        uint8_t option = USART_GetChar();
        handleMenuOption(option);
    } 
    return 0;
}