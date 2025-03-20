/*
 * LWM_MSSY.c
 *
 * Created: 6.4.2017 15:42:46
 * Author : Krajsa
 */ 

/*- Includes ---------------------------------------------------------------*/
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"
#include "main.h"
#include "at30tse758.h"

/*- Definitions ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
#define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
#define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
} AppState_t;

/*- Prototypes -------------------------------------------------------------*/
static void appSendData(void);

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;
void i2cInit(void);
void i2cStart(void);
void i2cStop(void);
void i2cWrite(uint8_t data);
uint8_t i2cReadACK(void);
uint8_t i2cReadNACK(void);
uint8_t i2cGetStatus(void);

uint8_t at30_setPrecision(uint8_t prec);
float at30_readTemp(void);

/*- Implementations --------------------------------------------------------*/




/*************************************************************************//**
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
appDataReqBusy = false;
(void)req;
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void)
{
if (appDataReqBusy || 0 == appUartBufferPtr)
return;

memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

appDataReq.dstAddr = 1-APP_ADDR;
appDataReq.dstEndpoint = APP_ENDPOINT;
appDataReq.srcEndpoint = APP_ENDPOINT;
appDataReq.options = NWK_OPT_ENABLE_SECURITY;
appDataReq.data = appDataReqBuffer;
appDataReq.size = appUartBufferPtr;
appDataReq.confirm = appDataConf;
NWK_DataReq(&appDataReq);

appUartBufferPtr = 0;
appDataReqBusy = true;
}
/*- Implementations --------------------------------------------------------*/
static void appSendDataRecv(void)
{
	static char ack = 06;
	appDataReq.dstAddr = 1-APP_ADDR;
	appDataReq.dstEndpoint = APP_ENDPOINT;
	appDataReq.srcEndpoint = APP_ENDPOINT;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = &ack;
	appDataReq.size = sizeof(char);
	appDataReq.confirm = appDataConf;
	NWK_DataReq(&appDataReq);

	appUartBufferPtr = 0;
	appDataReqBusy = true;
}

static void appSendDataTemp(void)
{
	static char ack = 59;
	appDataReq.dstAddr = 1-APP_ADDR;
	appDataReq.dstEndpoint = APP_ENDPOINT;
	appDataReq.srcEndpoint = APP_ENDPOINT;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = &ack;
	appDataReq.size = sizeof(char);
	appDataReq.confirm = appDataConf;
	NWK_DataReq(&appDataReq);

	appUartBufferPtr = 0;
	appDataReqBusy = true;
}


/*************************************************************************//**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
for (uint16_t i = 0; i < bytes; i++)
{
uint8_t byte = HAL_UartReadByte();

if (appUartBufferPtr == sizeof(appUartBuffer))
appSendData();

if (appUartBufferPtr < sizeof(appUartBuffer))
appUartBuffer[appUartBufferPtr++] = byte;
}

SYS_TimerStop(&appTimer);
SYS_TimerStart(&appTimer);
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer)
{
appSendData();
(void)timer;
}

/*************************************************************************//**
*****************************************************************************/
/*
static bool appDataInd(NWK_DataInd_t *ind)
{
	for (uint8_t i = 0; i < ind->size; i++)
	HAL_UartWriteByte(ind->data[i]);
	return true;
}
*/

static bool appDataInd(NWK_DataInd_t *ind)
{
	if ((ind->data[0] == 06 && ind->size == 1) || (ind->data[0] == 79 && ind->data[1] == 75 && ind->size == 2)) {
		HAL_UartWriteByte('|');
		} else {
		for (uint8_t i = 0; i < ind->size; i++) {
			HAL_UartWriteByte(ind->data[i]);
		}
		// Odeslání potvrzení "OK"
		NWK_DataReq_t okReq;
		uint8_t okData[] = { 'O', 'K' };

		okReq.dstAddr = ind->srcAddr;
		okReq.dstEndpoint = APP_ENDPOINT;
		okReq.srcEndpoint = APP_ENDPOINT;
		okReq.options = NWK_OPT_ENABLE_SECURITY;
		okReq.data = okData;
		okReq.size = sizeof(okData);
		okReq.confirm = appDataConf;
		NWK_DataReq(&okReq);
	}
	return true;
}

static SYS_Timer_t appTimerMOJ;
static void appTimerHandlerMOJ(SYS_Timer_t *timer)
{
	// handle timer event
	appSendDataTemp();
	
	//If (timeToStop)
	//SYS_TimerStop(timer);
}
static void startTimerMOJ(void)
{
	appTimerMOJ.interval = 2000;
	appTimerMOJ.mode = SYS_TIMER_PERIODIC_MODE;
	appTimerMOJ.handler = appTimerHandlerMOJ;
	SYS_TimerStart(&appTimerMOJ);
}


/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
NWK_SetAddr(APP_ADDR);
NWK_SetPanId(APP_PANID);
PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
PHY_SetBand(APP_BAND);
PHY_SetModulation(APP_MODULATION);
#endif
PHY_SetRxState(true);

NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

HAL_BoardInit();

appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
appTimer.mode = SYS_TIMER_INTERVAL_MODE;
appTimer.handler = appTimerHandler;

i2cInit();
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
switch (appState)
{
case APP_STATE_INITIAL:
{
appInit();
appState = APP_STATE_IDLE;
} break;

case APP_STATE_IDLE:
break;

default:
break;
}
}

void i2cInit(void){
	/* SCL freq 400kHz
	TWSR = 0x00; //preddelicka 1
	TWBR = 0x02; 
	*/
	TWCR = 1<<TWEN; //TWI enable
}
void i2cStart(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) == 0);  // waits for interrupt flag to take effect
}

void i2cStop(void){
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}
void i2cWrite(uint8_t data){
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) == 0);
}
uint8_t i2cReadACK(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}
uint8_t i2cReadNACK(void){
	TWCR = (1<<TWINT)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}
uint8_t i2cGetStatus(void){
	uint8_t status;
	status = TWSR & 0xF8; //status mask 0xF8 ~ 11111000
	return status;
}

uint8_t at30_setPrecision(uint8_t prec){
	uint_fast16_t config_register = 0;
	config_register |= (uint16_t)(prec << R0);
	i2cStart();
	i2cWrite(TempSensorAddrW);
	if (i2cGetStatus() != 0x18){
		UART_SendString("Error 18 \n\r");
		return 0;
	};
	i2cWrite(Temp_configRegister);
	if (i2cGetStatus() != 0x28){
		UART_SendString("Error 28 \n\r");
		return 0;
	};
	i2cWrite((uint8_t)(config_register>>8));
	if (i2cGetStatus() != 0x28){
		UART_SendString("Error 28 \n\r");
		return 0;
	};
	i2cWrite((uint8_t)(config_register));
	if (i2cGetStatus() != 0x28){
		UART_SendString("Error 28 \n\r");
		return 0;
	};
	i2cStop();
	return 1;
}

float at30_readTemp(void){
	volatile uint8_t buffer[2];
	volatile int16_t teplotaTMP;
	
	i2cStart();
	i2cWrite(TempSensorAddrW);
	if (i2cGetStatus() != 0x18){
		UART_SendString("Error 18 \n\r");
		return 0;
	};
	i2cWrite(Temp_tempRegister);
	if (i2cGetStatus() != 0x28){
		UART_SendString("Error 28 \n\r");
		return 0;
	};
	i2cStop();
	
	i2cStart();
	if (i2cGetStatus() != 0x08){
		UART_SendString("Error 08 \n\r");
		return 0;
	};
	i2cWrite(TempSensorAddrR);
	if (i2cGetStatus() != 0x40){
		UART_SendString("Error 40 \n\r");
		return 0;
	};
	buffer[0] = i2cReadACK();
	if (i2cGetStatus() != 0x50){
		UART_SendString("Error 50 \n\r");
		return 0;
	};
	buffer[1] = i2cReadNACK();
	if (i2cGetStatus() != 0x58){
		UART_SendString("Error 58 \n\r");
		return 0;
	};
	i2cStop();
	teplotaTMP = (buffer[0]<<8|buffer[1]);
	return (float)teplotaTMP/256;
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
SYS_Init();
HAL_UartInit(38400);
HAL_UartWriteByte('a');
startTimerMOJ();

while (1)
{
SYS_TaskHandler();
HAL_UartTaskHandler();
APP_TaskHandler();
}
}
