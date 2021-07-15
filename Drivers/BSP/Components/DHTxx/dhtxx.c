/**
  ******************************************************************************
  * @file    dhtxx.c
  * @brief   This file includes the HAL/LL driver for DHT11/22 Humidity
  * 		 Temperature Sensors
  ******************************************************************************
  */
#include "dhtxx.h"

/**
  * @brief  The internal function is used as gpio pin mode
  * @param  DHT		DHTxx_Drv_t
  * @param  Mode	Input or Output
  */
static void Set_Pin_Mode(DHTxx_Drv_t *DHT, PinMode Mode)
{
#ifdef LL_Driver
	if(Mode == Input)
	{
		LL_GPIO_SetPinMode(DHT->DataPort, DHT->DataPin, LL_GPIO_MODE_INPUT);
	}else{
		LL_GPIO_SetPinMode(DHT->DataPort, DHT->DataPin, LL_GPIO_MODE_OUTPUT);
	}
#else
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT->DataPin;
	if(Mode == Input)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}else{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	}
	HAL_GPIO_Init(DHT->DataPort, &GPIO_InitStruct);
#endif
}

/**
  * @brief  The internal function is used as gpio pin level
  * @param  DHT		DHTxx_Drv_t
  * @param  Mode	Level: Set/High = 1, Reset/Low = 0
  */
static void Set_Pin_Level(DHTxx_Drv_t *DHT, uint8_t Level)
{
#ifdef LL_Driver
	if(Level == 1)
	{
		LL_GPIO_SetOutputPin(DHT->DataPort, DHT->DataPin);
	}else{
		LL_GPIO_ResetOutputPin(DHT->DataPort, DHT->DataPin);
	}
#else
	HAL_GPIO_WritePin(DHT->DataPort, DHT->DataPin, Level);
#endif
}

/**
  * @brief  The internal function is used to read data pin
  * @retval Pin level status
  * @param  DHT		DHTxx_Drv_t
  */
static uint8_t Bus_Read(DHTxx_Drv_t *DHT)
{
#ifdef LL_Driver
	return ((DHT->DataPort->IDR & DHT->DataPin) != 0x00U) ? 1 : 0;
#else
	return HAL_GPIO_ReadPin(DHT->DataPort, DHT->DataPin);
#endif
}

/**
  * @brief  The internal function is used to check sensor response
  * @param  DHT		DHTxx_Drv_t
  * @retval OK = 1, Failed = -1
  */
static uint8_t DHT_Check_Response(DHTxx_Drv_t *DHT)
{
	uint8_t Response = 0;

	/* Set Data pin as Output */
	Set_Pin_Mode(DHT, Output);
	/* Reset Data Pin */
	Set_Pin_Level(DHT, 0);

	/* Delay waiting
	 * DHT22 = 1.2ms(give extra delay)
	 * DHT11 = 18ms
	 */
	DwtDelay_us((DHT->Type == DHT22) ? 1500 : 18000);

	/* Set Data Pin */
	Set_Pin_Level(DHT, 1);
	/* Delay 20us */
	DwtDelay_us(20);
	/* Set Data pin as Input */
	Set_Pin_Mode(DHT, Input);
	/* Delay 40us */
	DwtDelay_us(40);

	if(!Bus_Read(DHT))
	{
		DwtDelay_us(80);
		/* if the pin is high, response is ok */
		Response = (Bus_Read(DHT)) ? 1 : -1;
	}

	/* Wait for the pin to go reset */
	while(Bus_Read(DHT)) {};

	return Response;
}

/**
  * @brief  The internal function is used to read sensor data and return as byte
  * @param  DHT		DHTxx_Drv_t
  */
static uint8_t DHT_Read(DHTxx_Drv_t *DHT)
{
	uint8_t i, j;
	for(j = 0; j < 8; j++)
	{
		/* Wait for pin to go high */
		while(!Bus_Read(DHT));
		/* Delay 40us */
		DwtDelay_us(40);

		/* If the pin go reset */
		if(!Bus_Read(DHT))
		{
			i &= ~(1 << (7 - j));   // write 0
		}else{
			i |= (1 << (7 - j));  // if the pin is high, write 1
		}

		/* Wait the pin go reset */
		while(Bus_Read(DHT));
	}
	return i;
}

/**
  * @brief  The function is used as get temperature and humidity and return in
  * 		data structure
  * @param  DHT		DHTxx_Drv_t
  */
void DHT_GetData(DHTxx_Drv_t *DHT)
{
	if(DHT_Check_Response(DHT))
	{
		DHT->Data.Rh1 = DHT_Read(DHT);
		DHT->Data.Rh2 = DHT_Read(DHT);
		DHT->Data.Tp1 = DHT_Read(DHT);
		DHT->Data.Tp2 = DHT_Read(DHT);
		DHT->Data.Sum = DHT_Read(DHT);

		/* Check if data valid */
		if(DHT->Data.Sum == (DHT->Data.Rh1 + DHT->Data.Rh2 + DHT->Data.Tp1
				+ DHT->Data.Tp2))
		{
			DHT->Temperature = ((DHT->Data.Tp1 << 8) | DHT->Data.Tp2) / 10.0;
			DHT->Humidity = ((DHT->Data.Rh1 << 8) | DHT->Data.Rh2) / 10.0;
		}
	}
}
