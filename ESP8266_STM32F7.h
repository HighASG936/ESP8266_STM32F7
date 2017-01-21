#ifndef ESP8266_STM32F7_H
#define ESP8266_STM32F7_H

#include "Terminal_Uart.h"

#define NombreIndex	10
#define PassIndex		26

void Wifi_Inicializar(UART_HandleTypeDef UART);
void Wifi_EnviarATComand(uint8_t * Comando);
void Wifi_GetStatus (void);
void Wifi_Conectar(uint8_t * Name, uint8_t * Password);
void Wifi_Atencion(void);

uint8_t ATConectar[] = 
{
	'A','T','+','C','W','J','A','P','=',
	'"',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'"',',',
	'"',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'"'};

typedef struct
{
	UART_HandleTypeDef WifiUart;
	struct
	{
		void (*getStatus)(void);
		void (*Atencion)(void);
		void (*ATComand)(uint8_t * Comando);
		void (*Conectar)(uint8_t * Name, uint8_t * Password);

	};
}eWifi;

eWifi	gsWifi;

void Wifi_EnviarATComand(uint8_t * Comando)
{
	uint8_t *u8ATComand = Comando;
	while(*u8ATComand != 0x00) 
	{
	 	HAL_UART_Transmit(&gsWifi.WifiUart , (uint8_t *)u8ATComand,1,100);
		u8ATComand++;
	}	
	HAL_UART_Transmit(&gsWifi.WifiUart ,(uint8_t *) "\r\n",2,100);
	while (Terminal_Uart_Recibir(&gsWifi.WifiUart) != UartIdle);
}


void Wifi_GetStatus (void)
{
		gsWifi.ATComand((uint8_t *)"AT\r\n");
		while (Terminal_Uart_Recibir(&gsWifi.WifiUart) != UartIdle);
}


void Wifi_Atencion(void)
{
	Terminal_Uart_Atencion(gsWifi.WifiUart);
}

void Wifi_Conectar(uint8_t * Name, uint8_t * Password)
{
	uint8_t	u8ConectarIndex = NombreIndex;
	uint8_t u8ParametroIndex = 0;
	while(Name[u8ParametroIndex] != 0x00)
	{
		ATConectar[u8ConectarIndex] = Name[u8ParametroIndex];
		u8ConectarIndex++;
		u8ParametroIndex++;
	}
	u8ConectarIndex = PassIndex;
	u8ParametroIndex = 0;
	while(Password[u8ParametroIndex] != 0x00)
	{
		ATConectar[u8ConectarIndex] = Password[u8ParametroIndex];
		u8ConectarIndex++;
		u8ParametroIndex++;
	}
	Wifi_EnviarATComand(ATConectar);
	uint8_t i;
		printf("\r\nCONECTANDO...\r\n");
	for(i=0;i<200;i++)
	{
	while (Terminal_Uart_Recibir(&gsWifi.WifiUart) != UartIdle);
	}
}

void Wifi_Inicializar(UART_HandleTypeDef UART)
{
	gsWifi.WifiUart 			= 	UART;
	gsWifi.getStatus 			= 	Wifi_GetStatus;
	gsWifi.Atencion				=		Wifi_Atencion;
	gsWifi.ATComand				=   Wifi_EnviarATComand;
	gsWifi.Conectar				=		Wifi_Conectar;
	Terminal_Uart_Inicializar();
	gsWifi.getStatus();
}

#endif
