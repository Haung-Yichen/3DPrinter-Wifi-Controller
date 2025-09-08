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
#include "usart.h"
#include "cmdList.h"
#include "cmdHandler.h"
#include "printerController.h"
#include "Fatfs_SDIO.h"

#define ESP32_USART_PORT		 huart2              //正確是huart2 先用huart1調試
#define ESP32_OK				 "ok\n"                //用於與esp32同步狀態
#define ESP32_DISCONNECTED		 "wifi disconnected" //esp32 wifi異常會發送
#define ESP32_OVER				 0                   //用於檢查是否收到CMD_Transmisson_Over
#define rXBUF_SIZE				 (300)
#define FILENAME_SIZE			 20                  //用於創建檔案時規範
#define SHA256_HASH_SIZE         256
#define SD_RTY_TIMES			 5                   //sd寫檔重試次數
#define WAIT_ESP32_READY_TIMEOUT 10                  //最大等待ESP32初始化時間

typedef enum {
	ESP32_INIT = 0,
	ESP32_IDLE,
	ESP32_BUSY
} ESP32_STATE_TypeDef;

extern SemaphoreHandle_t traOverSemaphore; //傳檔完成信號量
extern SemaphoreHandle_t staPriSemaphore;  //開始列印信號量
extern SemaphoreHandle_t rxSemaphore;      //UART啟動解析信號量
extern QueueHandle_t cmdQueue;             //存放命令


// 內部狀態與緩衝區
extern volatile uint16_t rxLen;
extern uint8_t *ResBuf;                 //回調函數傳回結果緩衝區
extern char rxBuf[rXBUF_SIZE];        //接收緩衝區
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

/**
 * @brief 回傳查詢的資料給ESP32
 */
void ESP32_ReturnClbkRes(void);

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
#endif /* _ESP32_H_ */
