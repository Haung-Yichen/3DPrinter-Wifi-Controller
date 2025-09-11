/**
  ******************************************************************************
  * @file    cmox_low_level.c
  * @author  MCD Application Team
  * @brief   This file contains Low level functions for CMOX initialization
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "cmox_init.h"
#include "cmox_low_level.h"

// /* STM32F103 系列 HAL 標頭 */
// #include "stm32f1xx_hal.h"

/* 如果你不使用 HAL，可以註解掉上面的 include 並直接使用暫存器操作 */

/**
  * @brief          CMOX library low level initialization
  * @param          pArg User defined parameter that is transmitted from initialize service
  * @retval         Initialization status: @ref CMOX_INIT_SUCCESS / @ref CMOX_INIT_FAIL
  */
cmox_init_retval_t cmox_ll_init(void *pArg)
{
  (void)pArg;
  
  /* 確保 CRC 時鐘已啟用以獲得最佳密碼學處理效能 */
  /* 注意：STM32F103 沒有硬體 CRC 單元，但 CMOX 仍可正常運作 */
  /* 如果使用 HAL： */
  /* __HAL_RCC_CRC_CLK_ENABLE(); */  /* 在 STM32F103 上這行會編譯錯誤，因為沒有 CRC */
  
  /* STM32F103 沒有硬體 CRC，CMOX 會使用軟體實現，功能完全正常 */
  /* 不需要額外的時鐘配置 */
  
  return CMOX_INIT_SUCCESS;
}

/**
  * @brief          CMOX library low level de-initialization
  * @param          pArg User defined parameter that is transmitted from finalize service
  * @retval         De-initialization status: @ref CMOX_INIT_SUCCESS / @ref CMOX_INIT_FAIL
  */
cmox_init_retval_t cmox_ll_deInit(void *pArg)
{
  (void)pArg;
  /* 不要關閉 CRC 時鐘，避免影響其他使用 CRC 的軟體部分 */
  return CMOX_INIT_SUCCESS;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
