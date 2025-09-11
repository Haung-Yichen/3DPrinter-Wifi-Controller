#include "cmsis_os.h"
#include <fileTask.h>

osThreadId_t gcodeRxTaskHandle = NULL;
SemaphoreHandle_t fileSemaphore = NULL;

const osThreadAttr_t gcodeTask_attributes = {
	.name = "Gcode_Rx_Task",
	.stack_size = 128 * 32,
	.priority = (osPriority)osPriorityHigh5,
};

char fileBuf[FILE_BUF_SIZE] = {0};
volatile uint16_t fileLen = 0;
volatile bool delete = false;

void Gcode_RxHandler_Task(void *argument) {
	FIL tmpFile;
	FRESULT f_res;
	UINT fnum;

	char filename[FILENAME_SIZE] = {0};
	bool should_exit = false;
	bool received_data = false;
	bool done = false;

	fileSemaphore = xSemaphoreCreateBinary();

	printf("%-20s %-30s free heap : %d bytes \r\n",
		   "[fileTask.c]",
		   "Gcode_RxHandler_Task created!",
		   xPortGetFreeHeapSize());
	if (argument == NULL) {
		printf("%-20s argument is NULL\r\n", "[fileTask.c]");
		vTaskDelete(NULL);
	}

	memset(filename, 0, FILENAME_SIZE);
	strcpy(filename, argument);
	printf("%-20s creating %s... \r\n", "[fileTask.c]", filename);

	f_res = f_open(&tmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if (f_res != FR_OK) {
		printf("%-20s %-20s \r\n", "[fileTask.c]", "Failed to open file:");
		printf_fatfs_error(f_res);
		vTaskDelete(NULL);
	}

	// 清空檔案
	f_res = f_truncate(&tmpFile);
	if (f_res != FR_OK) {
		printf("%-20s %-30s %d \r\n", "[fileTask.c]", "Failed to truncate file:", f_res);
		f_close(&tmpFile); // 關閉檔案
		vTaskDelete(NULL);
	}
	printf("%-20s File had content and was truncated.\r\n", "[fileTask.c]");
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();

	while (1) {
		if (should_exit == true || delete == true) {
			f_close(&tmpFile);
			if (fileSemaphore != NULL) {
				vSemaphoreDelete(fileSemaphore);
				fileSemaphore = NULL;
			}
			delete = false;
			vTaskDelete(NULL);
		}
		//==================開始接收==================//
		received_data = (pdTRUE == xSemaphoreTake(fileSemaphore, pdMS_TO_TICKS(2000)));
		if (received_data) {
			// 正常接收邏輯
			// f_res = f_write(&tmpFile, fileBuf, fileLen, &fnum);
			// if (f_res != FR_OK) {
			// 	printf("%-20s SD write error\r\n", "[fileTask.c]");
			// 	printf_fatfs_error(f_res);
			// 	should_exit = true;
			// }
			// f_sync(&tmpFile);
			// printf("%-20s %s", "[fileTask.c]", fileBuf);
			printf("%s", fileBuf);
			memset(fileBuf, 0, FILE_BUF_SIZE);
		} else {
			continue;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	} // while
} // task
