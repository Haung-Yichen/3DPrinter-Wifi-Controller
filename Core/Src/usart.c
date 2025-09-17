/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>

#include "portmacro.h"
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART1 init function */
void MX_USART1_UART_Init(void) {
	huart1.Instance = USART1;
	huart1.Init.BaudRate = DEBUG_USART_BPS;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
}

/* USART2 init function */
void MX_USART2_UART_Init(void) {
	huart2.Instance = USART2;
	huart2.Init.BaudRate = ESP32_USART_BPS;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
}
void MX_USART3_UART_Init(void) {
	huart3.Instance = USART3;
	huart3.Init.BaudRate = PRINTER_USART_BPS;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (uartHandle->Instance == USART1) {
		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		hdma_usart1_tx.Instance = DMA1_Channel4;
		hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart1_tx.Init.Mode = DMA_NORMAL;
		hdma_usart1_tx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_usart1_tx);

		hdma_usart1_rx.Instance = DMA1_Channel5;
		hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart1_rx.Init.Mode = DMA_NORMAL;
		hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle, hdmarx, hdma_usart1_rx);

		HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	} else if (uartHandle->Instance == USART2) {
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		hdma_usart2_tx.Instance = DMA1_Channel7;
		hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart2_tx.Init.Mode = DMA_NORMAL;
		hdma_usart2_tx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK) {
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_usart2_tx);

		hdma_usart2_rx.Instance = DMA1_Channel6;
		hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart2_rx.Init.Mode = DMA_NORMAL;
		hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK) {
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle, hdmarx, hdma_usart2_rx);
		HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	} else if (uartHandle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {
	if (uartHandle->Instance == USART1) {
		__HAL_RCC_USART1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
		HAL_DMA_DeInit(uartHandle->hdmatx);
		HAL_DMA_DeInit(uartHandle->hdmarx);
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	} else if (uartHandle->Instance == USART2) {
		__HAL_RCC_USART2_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
		HAL_DMA_DeInit(uartHandle->hdmarx);
		HAL_NVIC_DisableIRQ(USART2_IRQn);
	} else if (uartHandle->Instance == USART3) {
		__HAL_RCC_USART3_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
		HAL_NVIC_DisableIRQ(USART3_IRQn);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TXE);
		__HAL_UNLOCK(&huart1); // 解鎖 UART
	} else if (huart->Instance == USART2) {
		__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
		__HAL_UNLOCK(&huart2);
	}
}

/* USER CODE BEGIN 1 */
// 重定向 printf 到 USART1
int _write(int fd, char *ptr, int len) {
	HAL_UART_Transmit(&DEBUG_USART_PORT, (uint8_t *) ptr, len, HAL_MAX_DELAY);
	return len;
}

/**
 * @brief 向網頁端傳送字串
 * @param str
 * @return
 */
HAL_StatusTypeDef UART_SendString_DMA(UART_HandleTypeDef* huart, const char *str) {
	if (str == NULL || strlen(str) == 0) {
		return HAL_ERROR;
	}
	static uint8_t retry = 0;
	const uint8_t maxRetries = 3;

	char tempBuffer[64];
	if (strlen(str) >= sizeof(tempBuffer)) {
		printf("%-20s String too long for DMA buffer\r\n", "[usart.c]");
		return HAL_ERROR;
	}
	strcpy(tempBuffer, str);

	retry = 0;
	while (retry <= maxRetries) {
		if (HAL_UART_GetState(huart) == HAL_UART_STATE_READY) {
			if (HAL_UART_Transmit_DMA(huart, (uint8_t *)tempBuffer, strlen(tempBuffer)) == HAL_OK) {
				retry = 0;
				return HAL_OK;
			} else {
				printf("\033[31m%-20s DMA transmission failed\033[0m\r\n", "[usart.c]");
				return HAL_ERROR;
			}
		} else {
			printf("%-20s UART busy, retry %d/%d\r\n", "[usart.c]", retry, maxRetries);
			osDelay(1);
			retry++;
			if (retry > maxRetries) {
				HAL_UART_Transmit(huart, (uint8_t *)tempBuffer, strlen(tempBuffer), HAL_MAX_DELAY);
				printf("\033[31m%-20s UART busy, will transmit directly\033[0m\r\n", "[usart.c]");
				return HAL_ERROR;
			}
		}
	}

	return HAL_ERROR;
}
/* USER CODE END 1 */
