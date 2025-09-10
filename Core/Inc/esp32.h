/**
 * @file    esp32.h
 * @author  HAUNG YI CHEN
 * @date    2025/06/24
 * @brief   該模組負責處理解析ESP32傳來的資料
 *		    並實作所有命令的回調函數，回調函數
 *		    再去呼叫真正執行該邏輯的函數。
 */

#ifndef _ESP32_H_
#define _ESP32_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "Fatfs_SDIO.h"
#include "usart.h"

#include "cmdList.h"
#include "cmdHandler.h"
#include "printerController.h"
#include "fileTask.h"


#define ESP32_USART_PORT		 huart1              //正確是huart2 先用huart1調試
#define ESP32_OK				 "ok\n"                //用於與esp32同步狀態
#define ESP32_DISCONNECTED		 "wifi disconnected" //esp32 wifi異常會發送
#define ESP32_OVER				 0                   //用於檢查是否收到CMD_Transmisson_Over
#define RXBUF_SIZE				 300
#define CMD_BUF_SIZE		     300
#define WAIT_ESP32_READY_TIMEOUT 10                  //最大等待ESP32初始化時間
#define ESP32_RECV_DELAY         100

typedef enum {
	ESP32_INIT = 0,
	ESP32_IDLE,
	ESP32_BUSY
} ESP32_STATE_TypeDef;

extern SemaphoreHandle_t cmdSemaphore;        //UART啟動解析信號量
extern SemaphoreHandle_t recvSemaphore;       //通知可以開始接收
// extern QueueHandle_t cmdQueue;             //存放命令

// 內部狀態與緩衝區
extern volatile uint16_t rxLen;
extern char rxBuf[RXBUF_SIZE];             //uart接收緩衝區
extern char cmdBuf[CMD_BUF_SIZE];          //命令接收緩衝區
static ESP32_STATE_TypeDef currentState = ESP32_INIT;

/**
 * @brief 初始化 ESP32 模組，註冊回調函數，並等待直到esp32回報初始化完成
 */
void ESP32_Init(void);

/**
 * @brief 註冊回調函數
 */
void ESP32_RegCallback(void);

/**
 * @brief 傳回目前 ESP32 狀態
 */
ESP32_STATE_TypeDef ESP32_GetState(void);

/**
 * @brief 設定 ESP32 狀態
 */
void ESP32_SetState(ESP32_STATE_TypeDef state);

/**
 * @brief 解析UART資料，DMA中斷觸發
 */
void ESP32_RxHandler_Task(void *argument);

/************************************************
*                 定義回調函數                  *
************************************************/

/**
 * @brief 命令 : 準備接收dcode
 */
void StartTransmissionCmdHandler(const char *args, void *res);

/**
 * @brief 命令：傳輸結束（command 觸發器）
 */
void TransmissionOverCmdHandler(const char *args, void *res);

/**
 * @brief 命令： 設定檔名
 */
void SetFileNameCmdHandler(const char *args, void *res);

HAL_StatusTypeDef reportOK_2_ESP32(void);

#endif /* _ESP32_H_ */
