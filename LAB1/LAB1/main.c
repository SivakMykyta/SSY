/*
 * LAB1.c
 *
 * Created: 02.02.2020 9:01:38
 * Author : Ondra
 */ 

/************************************************************************/
/* INCLUDE                                                              */
/************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/delay.h>
#include "libs/libprintfuart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define ODECET
#define KONSTANTA 2

#define DIRECTION_UP 
#define DIRECTION_DOWN 
#define UPPER_CASE
#define NORMAL_CASE 
#define PROGRAM_ERROR "PROGRAM ERROR"

#define F_CPU 8000000UL

// F_CPU definovano primo v projektu!!! Debug->Properties->Toolchain->Symbols
/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

//musime vytvorit soubor pro STDOUT
FILE uart_str = FDEV_SETUP_STREAM(printCHAR, NULL, _FDEV_SETUP_RW);

int a = 10;
int b = 10;

unsigned char c = 255;
unsigned char d = 255;
uint16_t e;

int f = 24;

int g = 200;
char h[] = "Hodnota=";
char str1[80];
char str2[80];
char buffer [sizeof(int)*8+1];
char abc[80];


/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void board_init();

int generateField();

void capsLetters();

void printField();

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
void board_init(){
	UART_init(38400); //nastaveni rychlosti UARTu, 38400b/s
	stdout = &uart_str; //presmerovani STDOUT
}

/*int generateField(){
	int abcStart;
	
	#ifdef UPPER_CASE
		abcStart = 65;
	#endif
	#ifdef NORMAL_CASE
		abcStart = 97;
	#endif
	
	#ifndef (UPPER_CASE & NORMAL_CASE)
	printf(PROGRAM_ERROR);
	return 1;
	#endif
	
	int i;
	for (i = 0, i < 32, i++)
	{
		abc[i] = abcStart+i;
	}
	return 0;
}

void capsLetters();

void printField();*/

int main(void)
{ 	
	board_init();
	_delay_ms(1000);
	
	printf("Test_1_ukol_4\n\r");
	printf("a = %d \n\r", a);
	printf("b = %d \n\r", b);
	printf("Hello word\n\r");
	#ifndef ODECET
	a = a - KONSTANTA;
	#endif
	#ifndef ODECETT
	b = b - KONSTANTA;
	#endif
	printf("a = %d \n\r", a);
	printf("b = %d \n\r", b);
	
	printf("Test_1_ukol_5\n\r");
	e = (int)c + d; // e = 255 + 255 = 510
	printf("e = %d \n\r",e);
	
	printf("Test_1_ukol_6\n\r");
	f = f >> 3;
	printf("f = f >> 3: %d \n\r", f);
	f = f - 1;
	printf("f = f - 1: %d \n\r", f);
	f = f & 0x2;
	printf("f = f & 0x2; %d \n\r", f);
	
	printf("Test_1_ukol_7\n\r");
	strcpy(str1, h);
	strcat(str1, itoa(g, buffer,10));
	printf("%s \n\r",str1);
	sprintf(str2, "%s%d",h,g);
	printf("%s \n\r",str2);
	
	printf("Test_1_ukol_8\n\r");
	
	printf("Test_1_ukol_9\n\r");
	DDRG = 0b00000010;
	PORTG = 0b00000010;
	// equivalent hex notation
	//DDRG = 0x02;
	//PORTG = 0x02;
	// equivalent decimal notation
	//DDRG = 2;
	//PORTG = 2;
	while(1)
	{
		PORTG = 0b00000010;
		_delay_loop_2(0xFFFF);
		PORTG = 0b00000000;
		_delay_loop_2(0xFFFF);
	}
	

	/*printf("Hello word\n\r");
    _delay_ms(1000);
	int i=0;
    while (1) 
    {
	_delay_ms(10000);
	i++;
	printf("Test x = %d \n\r", i);
    }*/
}

