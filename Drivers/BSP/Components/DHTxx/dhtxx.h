/**
  ******************************************************************************
  * @file    dhtxx.h
  * @brief   This file contains all the constants parameters for the DHT11/22
  * 		 Humidity Temperature Sensors
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DHTXX_H
#define DHTXX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Driver Selection ----------------------------------------------------------*/
//#define LL_Driver

/* DHTxx Data Structure ------------------------------------------------------*/
typedef enum
{
  DHT11,
  DHT22
} SensorType;

typedef enum
{
  Input,
  Output
} PinMode;

typedef struct {
	uint8_t		Rh1;
	uint8_t		Rh2;
	uint8_t		Tp1;
	uint8_t		Tp2;
	uint16_t	Sum;
}DHTxx_Raw_t;

typedef struct
{
	GPIO_TypeDef	*DataPort;
	uint16_t		DataPin;
	SensorType		Type;
	DHTxx_Raw_t		Data;
	float 			Temperature;
	float 			Humidity;
}DHTxx_Drv_t;

/* DHTxx External Function ---------------------------------------------------*/
void DHT_GetData(DHTxx_Drv_t *DHT);

#ifdef __cplusplus
}
#endif

#endif /* DHTXX_H */
