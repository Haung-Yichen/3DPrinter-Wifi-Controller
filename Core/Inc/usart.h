/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "esp32.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
#define DEBUG_USART_PORT huart1
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

void MX_USART2_UART_Init(void);

void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief 使用 DMA 傳送 C 字串 (null-terminated)
 * @param str 傳入欲傳送的字串指標 (必須是以 '\0' 結尾的字串)
 * @retval HAL status
 */
HAL_StatusTypeDef UART_SendString_DMA(const char *str);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
