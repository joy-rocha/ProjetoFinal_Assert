/******************************************************************************/
/**
 * @file Bsp.h
 * @addtogroup Bsp
 * @{
 ******************************************************************************/

#ifndef BSP_H
#define BSP_H

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include "AssertTypes.h"
#include "stm32f4xx_hal.h"

/*******************************************************************************
 * CONFIGURACOES
 ******************************************************************************/

/*******************************************************************************
 * DEFINES PUBLICOS
 ******************************************************************************/
#define dBSP_LED_CHANNEL_1                                                     1
#define dBSP_LED_CHANNEL_2                                                     2
#define dBSP_LED_CHANNEL_3                                                     3

/*******************************************************************************
 * TIPOS DE DADOS PUBLICOS
 ******************************************************************************/

/*******************************************************************************
 * PROTOTIPOS PUBLICOS
 ******************************************************************************/
void Bsp_Init(void);
bool Bsp_GetSamplingFlag(void);
void Bsp_ClearSamplingFlag(void);
u16 Bsp_AdcRead(void);
void Bsp_PwmSetDuty(u8 channel, u8 duty);
bool Bsp_GetButtonExtiFlag(void);
void Bsp_ClearButtonExtiFlag(void);
void Bsp_StartDebounceTimer(void);
bool Bsp_GetDebounceTimeoutFlag(void);
void Bsp_ClearDebounceTimeoutFlag(void);
bool Bsp_UartRxHasData(void);
u8 Bsp_UartRxReadByte(void);

#endif /* BSP_H */

/** @} */
