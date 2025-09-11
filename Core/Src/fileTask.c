#include "cmsis_os.h"
#include <fileTask.h>

osThreadId_t gcodeRxTaskHandle = NULL;
SemaphoreHandle_t fileSemaphore = NULL;

const osThreadAttr_t gcodeTask_attributes = {
	.name = "Gcode_Rx_Task",
	.stack_size = 128 * 32,
	.priority = (osPriority)osPriorityHigh5,
};

char hashVal[100] = {0};
char fileBuf[FILE_BUF_SIZE] = {0};
volatile uint16_t fileLen = 0;
volatile bool delete = false;
// cmox_hash_handle_t* hashHandle = NULL;

/**
 * @brief 刪除任務 並自動刪除指針
 */
static void deleteTask();

void Gcode_RxHandler_Task(void *argument) {
	FIL tmpFile;
	FRESULT f_res;
	UINT fnum;

	// cmox_sha256_handle_t sha256Handle;
	uint32_t i = 0; // 檔案接收行數計數器
	char filename[FILENAME_SIZE] = {0};
	bool should_exit = false;
	bool received_data = false;
	bool done = false;

	// 初始化cmox sha256
	// hashHandle = cmox_sha256_construct(&sha256Handle);
	fileSemaphore = xSemaphoreCreateBinary();

	printf("%-20s %-30s free heap : %d bytes \r\n",
	   "[fileTask.c]",
	   "Gcode_RxHandler_Task created!",
	   xPortGetFreeHeapSize());

	// 初始化cmox 檢查資源有效值
	// if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS) {
	// 	printf("%-20s Failed to initialize CMOX library\r\n", "[fileTask.c]");
	// 	deleteTask();
	// } else if (hashHandle == NULL) {
	// 	printf("%-20s Failed to init SHA256 handle\r\n", "[fileTask.c]");
	// 	deleteTask();
	// } else if (fileSemaphore == NULL) {
	// 	printf("%-20s failed to creat fileSemaphore!\r\n", "[fileTask.c]");
	// 	deleteTask();
	// } else if (argument == NULL) {
	// 	printf("%-20s argument is NULL\r\n", "[fileTask.c]");
	// 	deleteTask();
	// }

	memset(filename, 0, FILENAME_SIZE);
	strcpy(filename, argument);
	printf("%-20s creating %s... \r\n", "[fileTask.c]", filename);

	f_res = f_open(&tmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if (f_res != FR_OK) {
		printf("%-20s %-20s \r\n", "[fileTask.c]", "Failed to open file:");
		printf_fatfs_error(f_res);
		deleteTask();
	}
	// 清空檔案
	f_res = f_truncate(&tmpFile);
	if (f_res != FR_OK) {
		printf("%-20s %-30s %d \r\n", "[fileTask.c]", "Failed to truncate file:", f_res);
		f_close(&tmpFile); // 關閉檔案
		deleteTask();
	}
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();

	while (1) {
		if (should_exit == true || delete == true) {
			f_close(&tmpFile);
			delete = false;
			deleteTask();
		}
		//==================開始接收==================//
		received_data = (pdTRUE == xSemaphoreTake(fileSemaphore, pdMS_TO_TICKS(2000)));
		if (received_data) {
			i++;
			// 正常接收邏輯
			f_res = f_write(&tmpFile, fileBuf, fileLen, &fnum);
			if (f_res != FR_OK) {
				printf("%-20s SD write error\r\n", "[fileTask.c]");
				printf_fatfs_error(f_res);
				should_exit = true;
			}
			if (i >= 16) {
				i = 0;
				f_sync(&tmpFile);
			}

			printf("%s", fileBuf);
			memset(fileBuf, 0, FILE_BUF_SIZE);
		} else {
			continue;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	} // while
} // task

static void deleteTask() {
	if (fileSemaphore != NULL) {
		vSemaphoreDelete(fileSemaphore);
		fileSemaphore = NULL;
	}
	// } else if (hashHandle != NULL) {
	// 	cmox_hash_cleanup(hashHandle);
	// 	hashHandle = NULL;
	// }
	vTaskDelete(NULL);
}