#ifndef FILETASK_H
#define FILETASK_H

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ff.h"
#include "sha256.h"
#include "esp32.h"

#define FILENAME_SIZE			 20                  //用於創建檔案時規範
#define SHA256_HASH_SIZE         70
#define FILE_BUF_SIZE			 2048
#define SD_RTY_TIMES			 5                   //sd寫檔重試次數

extern SemaphoreHandle_t fileSemaphore; //用來通知是否收到檔案
extern osThreadId_t gcodeRxTaskHandle;
extern const osThreadAttr_t gcodeTask_attributes;

extern char hashVal[SHA256_HASH_SIZE];
extern char fileBuf[FILE_BUF_SIZE];
extern char filename[FILENAME_SIZE];
extern volatile uint16_t fileLen;
extern volatile bool delete;

/**
 * @brief 檔案接收任務，接收來自ESP32 UART的GCODE
 *		  並存入SD卡，須按照ESP32的狀態來接收
 * @param argument
 */
void Gcode_RxHandler_Task(void *argument);

/**
 * @brief 計算檔案sha256哈希值
 * @param _filename
 * @return
 */
void calFileHash(const char* _filename);


#endif //FILETASK_H
