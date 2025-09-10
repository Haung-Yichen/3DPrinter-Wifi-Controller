#include "cmsis_os.h"
#include <fileTask.h>


osThreadId_t gcodeRxTaskHandle = NULL;
SemaphoreHandle_t fileSemaphore = NULL;

const osThreadAttr_t gcodeTask_attributes = {
	.name = "Gcode_Rx_Task",
	.stack_size = 128 * 16,
	.priority = (osPriority)osPriorityHigh5,
};

char fileBuf[FILE_BUF_SIZE] = {0};
volatile uint8_t fileLen = 0;

void Gcode_RxHandler_Task(void *argument) {
	FIL tmpFile;
	FRESULT f_res;
	UINT fnum;

	char filename[FILENAME_SIZE];
	bool should_exit = false;
	bool received_data = false;
	bool done = false;

	if (argument == NULL) {
		printf("argument is NULL\r\n");
		osThreadTerminate(osThreadGetId());
	}
	strcpy(filename, argument);
	printf("creating %s...\r\n", filename);
	f_res = f_open(&tmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if (f_res != FR_OK) {
		printf("Failed to open file: %d\r\n", f_res);
		osThreadTerminate(osThreadGetId());
	}
	// 檢查檔案大小
	if (f_size(&tmpFile) > 0) {
		// 檔案有內容，清空檔案
		f_res = f_truncate(&tmpFile); // 清空檔案內容
		if (f_res != FR_OK) {
			printf("Failed to truncate file: %d\r\n", f_res);
			f_close(&tmpFile); // 關閉檔案
			osThreadTerminate(osThreadGetId());
		}
		printf("File had content and was truncated.\r\n");
	}
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();

	while (1) {
		if (should_exit == true) {
			f_close(&tmpFile);
			osThreadTerminate(osThreadGetId());
		}
		//==================開始接收==================//
		received_data = (pdTRUE == xSemaphoreTake(fileSemaphore, pdMS_TO_TICKS(2000)));
		if (received_data) {
			// 正常接收邏輯
			f_res = f_write(&tmpFile, fileBuf, fileLen, &fnum);
			if (f_res != FR_OK) {
				printf("SD write error\r\n");
				printf_fatfs_error(f_res);
				should_exit = true;
			}
			f_sync(&tmpFile);
			printf("%s", fileBuf);
			memset(fileBuf, 0, FILE_BUF_SIZE);
		} else {
			should_exit = true;
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	} // while
} // task
