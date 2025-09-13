/*********************************************************************
 * @file   printer_controller.h
 * @brief  印表機控制器
 * 為本項目核心模組，負責與印表機通訊，接收來自電腦端與esp32的命令，
 * 以及emWin GUI。每隔一段時間會主動獲取傳感器狀態，並提供接口讓外部訪問。
 *
 * @author Lin, YiChen
 * @date   2024.12.08
 *********************************************************************/

#ifndef _PRINTER_CONTROLLER_H_
#define _PRINTER_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "cmdHandler.h"
#include "cmdList.h"
#include "ff.h"

extern SemaphoreHandle_t priOverSemaphore; //列印完成信號量

void PC_init(void);

/**
 * @brief 註冊回調函數
 */
void PC_RegCallback(void);

void PC_PrintTask();

/************************************************
*                 定義回調函數                  *
************************************************/

/**
 * @brief 開始列印命令的處理函式
 */
void StartToPrintCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 暫停列印命令的處理函式
 */
void PausePrintingCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 停止列印命令的處理函式
 */
void StopPrintingCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 回到原點命令的處理函式
 */
void GoHomeCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 請求剩餘列印時間命令的處理函式
 */
void GetRemainingTimeCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 請求列印進度命令的處理函式
 */
void GetProgressCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 請求噴嘴溫度命令的處理函式
 */
void GetNozzleTempCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 請求熱床溫度命令的處理函式
 */
void GetBedTempCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 設置噴嘴溫度命令的處理函式
 */
void SetNozzleTempCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 設置熱床溫度命令的處理函式
 */
void SetBedTempCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 請求耗材重量命令的處理函式
 */
void GetFilamentWeightCmdHandler(const char *args, ResStruct_t* _resStruct);

/**
 * @brief 緊急停止命令的處理函式
 */
void EmergencyStopCmdHandler(const char *args, ResStruct_t* _resStruct);

//
// void

#ifdef __cplusplus
}
#endif

#endif /* _PRINTER_CONTROLLER_H_ */
