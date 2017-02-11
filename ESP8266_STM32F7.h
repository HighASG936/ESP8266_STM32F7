#ifndef ESP8266_STM32F7_H
#define ESP8266_STM32F7_H

#include "Terminal_Uart.h"

#define NombreIndex	10
#define PassIndex		26

uint8_t Wifi_Inicializar(UART_HandleTypeDef UART, GPIO_TypeDef * WifiPortBuzzer, uint16_t Wifi_Pin);
void Wifi_EnviarATComand(uint8_t * Comando);
void Wifi_GetStatus (void);
void Wifi_Conectar(uint8_t * Name, uint8_t * Password);
void Wifi_Atencion(void);

uint8_t ATConectar[] = 
{
	'A','T','+','C','W','J','A','P','=',
	'"',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'"',',',
	'"',0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,'"'
};

GPIO_InitTypeDef Wifi_InitStruct;		//Estructura del GPIO para la tarjeta ESP8266

enum
{
	Busy,
	Success,
	Idle
}WifiFuncStatus;
	
typedef union
{
	uint8_t all;
	
	struct
	{
		uint8_t Bit7 								:1;
		uint8_t Bit6 								:1;
		uint8_t Bit5 								:1;
		uint8_t Bit4 								:1;
		uint8_t Bit3 								:1;
		uint8_t Bit2 								:1;
		uint8_t Bit1 								:1;
		uint8_t InstructionThen 		:1;
	};

}eWifiFlags;
		
typedef struct
{
	UART_HandleTypeDef WifiUart;
	eWifiFlags Flags;
	struct
	{
		void (*getStatus)(void);
		void (*Atencion)(void);
		void (*ATComand)(uint8_t * Comando);
		void (*Conectar)(uint8_t * Name, uint8_t * Password);
	};
	
	
}eWifi;

eWifi	gsWifi;

//-------------------------------------------------------------------------------------------------------------------------
//Name: 
//
//Description:
//
//
//Parameters: 
//
//Return: 
//
//Author: Aurelio Siordia González
//Fecha: 20/01/2017
//-------------------------------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------------------------------
//Name: 
//
//Description:
//
//
//Parameters: 
//
//Return: 
//
//Author: Aurelio Siordia González
//Fecha: 20/01/2017
//-------------------------------------------------------------------------------------------------------------------------
void Wifi_GetStatus (void)
{
		gsWifi.ATComand((uint8_t *)"AT\r\n");
		while (Terminal_Uart_Recibir(&gsWifi.WifiUart) != UartIdle);
}

//-------------------------------------------------------------------------------------------------------------------------
//Name: 
//
//Description:
//
//
//Parameters: 
//
//Return: 
//
//Author: Aurelio Siordia González
//Fecha: 20/01/2017
//-------------------------------------------------------------------------------------------------------------------------
void Wifi_Atencion(void)
{
	Terminal_Uart_Atencion(gsWifi.WifiUart);	
	//Si se trata de ingresar un dato para transmitir
	if(gsTerminalUart.GetCharRx() == '>' && gsWifi.Flags.InstructionThen == false) 
	{
		gsTerminalUart.CharRx = 0x00;
		gsWifi.Flags.InstructionThen = true;
	}
	gsWifi.Flags.InstructionThen = false;
	gsTerminalUart.Flag.Recibiendo = false;
}

//-------------------------------------------------------------------------------------------------------------------------
//Name: 
//
//Description:
//
//
//Parameters: 
//
//Return: 
//
//Author: Aurelio Siordia González
//Fecha: 20/01/2017
//-------------------------------------------------------------------------------------------------------------------------
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
	uint16_t i;
	for(i=0;i<100;i++)
	{
	while (Terminal_Uart_Recibir(&gsWifi.WifiUart) != UartIdle);
	}
}

//-------------------------------------------------------------------------------------------------------------------------
//Name: 
//
//Description:
//
//
//Parameters: 
//
//Return: 
//
//Author: Aurelio Siordia González
//Fecha: 20/01/2017
//-------------------------------------------------------------------------------------------------------------------------
uint8_t Wifi_Inicializar(UART_HandleTypeDef UART, GPIO_TypeDef * WifiPortBuzzer, uint16_t Wifi_Pin)
{
	static uint8_t 	StatusWifiInicializar = 0;
	static uint32_t Counter  = 5000;
	
	if(StatusWifiInicializar == 0)
	{
		gsWifi.WifiUart 			= 	UART;
		gsWifi.getStatus 			= 	Wifi_GetStatus;
		gsWifi.Atencion				=		Wifi_Atencion;
		gsWifi.ATComand				=   Wifi_EnviarATComand;
		gsWifi.Conectar				=		Wifi_Conectar;
		
		//Iniciar módulo UART
		Terminal_Uart_Inicializar();
		
		// Inicializa Pin para activar tarjeta de Wifi
		Wifi_InitStruct.Pin 	= Wifi_Pin;
		Wifi_InitStruct.Mode	= GPIO_MODE_OUTPUT_PP;
		Wifi_InitStruct.Pull = GPIO_NOPULL;
		Wifi_InitStruct.Speed = GPIO_SPEED_LOW;
		
		HAL_GPIO_Init(WifiPortBuzzer, &Wifi_InitStruct);
		
		//Iniciar pin en reset
		HAL_GPIO_WritePin(WifiPortBuzzer, Wifi_Pin, GPIO_PIN_RESET);
		StatusWifiInicializar = ~StatusWifiInicializar;
		return(Busy);
	}
	// Lag antes de encender
	Counter--;
	if(Counter != 0) return(Busy);
	
	// Encender tarjeta de Wifi
	HAL_GPIO_WritePin(WifiPortBuzzer, Wifi_Pin, GPIO_PIN_SET);
	return(Success);
}
#endif
