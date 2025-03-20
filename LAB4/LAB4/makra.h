/*
 * makra.h
 *
 * Created: 15.02.2023 12:26:25
 *  Author: Ondra
 */ 


#ifndef MAKRA_H_
#define MAKRA_H_

#include <avr/io.h>

//Bitove operace
#define sbi(var, mask)  ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)  ((var) &= (uint8_t)~(1 << mask))
#define tbi(var,mask)	(var & (1 << mask))
#define xbi(var,mask)	((var)^=(uint8_t)(1 << mask))
#define sbiX(var, mask) (var|= (1 << mask))
//hardware

#define LED0ON cbi(PORTB,4)
#define LED0OFF sbi(PORTB,4)
#define LED0CHANGE xbi(PORTB,4)
#define LED1ON cbi(PORTB,5)
#define LED1OFF sbi(PORTB,5)
#define LED1CHANGE xbi(PORTB,5)
#define LED2ON cbi(PORTB,6)
#define LED2OFF sbi(PORTB,6)
#define LED2CHANGE xbi(PORTB,6)
#define LED3ON cbi(PORTE,3)
#define LED3OFF sbi(PORTE,3)
#define LED3CHANGE xbi(PORTE,3)








#endif /* MAKRA_H_ */