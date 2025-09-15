#include "printerController.h"

#include "esp32.h"


void PC_init(void) {
	PC_RegCallback();
}

void PC_RegCallback(void) {
	register_command(CMD_Start_To_Print, StartToPrintCmdHandler);
	register_command(CMD_Pause_Printing, PausePrintingCmdHandler);
	register_command(CMD_Stop_printing, StopPrintingCmdHandler);
	register_command(CMD_Go_Home, GoHomeCmdHandler);
	register_command(CMD_Get_Remainning_time, GetRemainingTimeCmdHandler);
	register_command(CMD_Get_Progress, GetProgressCmdHandler);
	register_command(CMD_Get_Nozzle_Temp, GetNozzleTempCmdHandler);
	register_command(CMD_Get_Bed_Temp, GetBedTempCmdHandler);
	register_command(CMD_Set_Nozzle_Temp, SetNozzleTempCmdHandler);
	register_command(CMD_Set_Bed_Temp, SetBedTempCmdHandler);
	register_command(CMD_GetFilament_Weight, GetFilamentWeightCmdHandler);
	register_command(CMD_Emergency_Stop, EmergencyStopCmdHandler);
}

void StartToPrintCmdHandler(const char *args, ResStruct_t* _resStruct) {
    FIL file;
    FRESULT f_res;
    char gcode_line[256] = {0};
    char printer_response[64] = {0};
	uint32_t line = 0;
    bool file_opened = false;

	if (strlen(filename) <= 0) {
		printf("%-20s no file selected\r\n", "[printerController.c]");
		return;
	}
	
    f_res = f_open(&file, filename, FA_READ);
    if (f_res != FR_OK) {
        printf("%-20s Failed to open file: %d\r\n", "[printerController.c]", f_res);
        return;
    }
	if (f_size(&file) <= 0) {
		printf("%-20s file has no content\r\n", "[printerController.c]");
		return;
	}
    file_opened = true;
    ESP32_SetState(ESP32_BUSY);

    while (f_gets(gcode_line, sizeof(gcode_line), &file) != NULL) {
    	line++;
        // 跳過空行和註解行
        if (strlen(gcode_line) == 0 || strchr(gcode_line, ';') != NULL) {
            continue;
        }
        // 發送 G-code 到印表機
        printf("Sending: %s", gcode_line);

        // HAL_StatusTypeDef uart_status = HAL_UART_Transmit(&huart3,
        //                                                  (uint8_t*)gcode_line,
        //                                                  strlen(gcode_line),
        //                                                  1000);
        //
        // if (uart_status != HAL_OK) {
        //     printf("UART transmission failed: %d\r\n", uart_status);
        //     break;
        // }

        // 等待印表機回復 "ok"
        // memset(printer_response, 0, sizeof(printer_response));

        // 使用阻塞接收等待 "ok" 回應
        // uart_status = HAL_UART_Receive(&huart3,
        //                               (uint8_t*)printer_response,
        //                               sizeof(printer_response) - 1,
        //                               5000);  // 5秒超時

        // if (uart_status == HAL_OK || uart_status == HAL_TIMEOUT) {
        //     // 確保字串結尾
        //     printer_response[sizeof(printer_response) - 1] = '\0';
        //
        //     // 檢查是否收到 "ok"
        //     if (strstr(printer_response, "ok") != NULL) {
        //         printf("Printer responded: %s", printer_response);
        //     } else {
        //         printf("Unexpected printer response: %s\r\n", printer_response);
        //         // 繼續執行，不中斷列印
        //     }
        // } else {
        //     printf("Failed to receive printer response: %d\r\n", uart_status);
        //     // 可選擇是否中斷列印
        //     break;
        // }
        // 清空 gcode_line 準備下一行
        memset(gcode_line, 0, sizeof(gcode_line));
    }
    printf("\r\n%-20s printTask completed! line: %d\r\n","[printerController.c]", line);
	f_close(&file);
    ESP32_SetState(ESP32_IDLE);
}

void PausePrintingCmdHandler(const char *args, ResStruct_t* _resStruct) {
	//通知印表機控制器暫停發送gcode
}

void StopPrintingCmdHandler(const char *args, ResStruct_t* _resStruct) {
	//清空列印計數器，印表機回原點
}

void GoHomeCmdHandler(const char *args, ResStruct_t* _resStruct) {
	UART_SendString_DMA(&DEBUG_USART_PORT, "G28\r\n");
}

void GetRemainingTimeCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作回傳剩餘列印時間邏輯
}

void GetProgressCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作回傳列印進度邏輯
}

void GetNozzleTempCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作回傳噴嘴溫度邏輯
}

void GetBedTempCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作回傳熱床溫度邏輯
}

void SetNozzleTempCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作設置噴嘴溫度邏輯
}

void SetBedTempCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作設置熱床溫度邏輯
}

void GetFilamentWeightCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作回傳耗材重量邏輯
}

void EmergencyStopCmdHandler(const char *args, ResStruct_t* _resStruct) {
	// TODO: 實作緊急停止邏輯
}
