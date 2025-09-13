#ifndef FILETASK_H
#define FILETASK_H

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ff.h"
#include "sha256.h"
#include "esp32.h"

#define FILENAME_SIZE			 20                  //用於創建檔案時規範
#define SHA256_HASH_SIZE         256
#define FILE_BUF_SIZE			 2048
#define SD_RTY_TIMES			 5                   //sd寫檔重試次數

extern SemaphoreHandle_t fileSemaphore; //用來通知是否收到檔案
extern osThreadId_t gcodeRxTaskHandle;
extern const osThreadAttr_t gcodeTask_attributes;

extern char hashVal[100];
extern char fileBuf[FILE_BUF_SIZE];
extern volatile uint16_t fileLen;
extern volatile bool delete;

/**
 * @brief 檔案接收任務，接收來自ESP32 UART的GCODE
 *		  並存入SD卡，須按照ESP32的狀態來接收
 *
 *		  --------------------狀態說明--------------------
 *		  1, CmdHandler收到"cStartTransmission"即動態創建本任務
 *		  2, PREPARE_FOR_RECEIVING:
 *				回傳ESP32_OK，等待UART IDLE IT(rxSemaphore)通知收完檔名後
 *				跳轉至HANDLE_FILENAME。
 *		  3, HANDLE_FILENAME:
 *				將CRXBuffer收到的"cSetFilename<filename>"命令中的檔名解析出來
 *				複製到filename，並在SD中開啟/建立檔案，然後把UART DMA接收緩衝區
 *				設為FRXBuffer，回傳ESP32_OK，之後跳轉至WAIT_FOR_NEXT_LINE。
 *				若開檔/建檔失敗則進死循環，未來再加入錯誤處理。
 *		  4, WAIT_FOR_NEXT_LINE:
 *				循環等待rxSemaphore，將FRXBuffer收到的GCODE一行行寫入
 *				當前檔案的每一行，之後清空FRXBuffer，繼續下一個循環直到
 *				收到"cTransmissonOver<hashVal>"後，代表檔案傳輸完成。
 *		  5, VERIFY_FILE:
 *				使用SHA256計算檔案哈希值，與收到的hashVal做比對，若相同代表檔案無誤，
 *				回傳ESP32_OK，若不一致，回傳"eFileBroken"，未來再加入錯誤處理，
 *				跳轉回PREPARE_FOR_RECEIVING，關檔結束該函式。
 *
 * @param argument
 */
void Gcode_RxHandler_Task(void *argument);


#endif //FILETASK_H
