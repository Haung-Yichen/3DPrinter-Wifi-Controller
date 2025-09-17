#include "cmsis_os.h"
#include <fileTask.h>

osThreadId_t gcodeRxTaskHandle = NULL;
SemaphoreHandle_t fileSemaphore = NULL;

const osThreadAttr_t gcodeTask_attributes = {
	.name = "Gcode_Rx_Task",
	.stack_size = 128 * 56,
	.priority = (osPriority) osPriorityHigh7,
};

char hashVal[SHA256_HASH_SIZE] = {0};
char fileBuf[FILE_BUF_SIZE] = {0};
char filename[FILENAME_SIZE] = {0};
FIL tmpFile;
volatile uint16_t fileLen = 0;
volatile bool delete = false;

/**
 * @brief 刪除任務 並自動刪除指針
 */
static void deleteTask();

void Gcode_RxHandler_Task(void *argument) {
	FRESULT f_res;
	UINT fnum;
	SHA256_CTX sha256_ctx;

	uint32_t stackHighWaterMark = 0;
	uint32_t i = 0; // 檔案接收行數計數器
	uint32_t fnumCount = 0;
	uint32_t timer = 0;
	char filename[FILENAME_SIZE] = {0};
	bool should_exit = false;
	bool received_data = false;
	bool sdErr = false;

	sha256_init(&sha256_ctx);
	fileSemaphore = xSemaphoreCreateBinary();

	printf("%-20s %-30s free heap: %d bytes \r\n",
	       "[fileTask.c]",
	       "Gcode_RxHandler_Task created!",
	       xPortGetFreeHeapSize());

	if (fileSemaphore == NULL) {
		printf("%-20s failed to creat fileSemaphore!\r\n", "[fileTask.c]");
		deleteTask();
	} else if (argument == NULL) {
		printf("%-20s argument is NULL\r\n", "[fileTask.c]");
		deleteTask();
	}

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
	timer = xTaskGetTickCount();
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();

	uint16_t cnt = 0;
	printf("%-20s %-20s \r\n", "[fileTask.c]", "Ready to receive.");


#ifdef DEBUG
	stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
#endif

	while (1) {
		//==================開始接收==================//
		received_data = (pdTRUE == xSemaphoreTake(fileSemaphore, pdMS_TO_TICKS(1000)));
		if (received_data && fileLen != 0) {
			cnt = 0;
			i++;
			// 正常接收邏輯
			f_res = f_write(&tmpFile, fileBuf, fileLen, &fnum);
			sha256_update(&sha256_ctx, fileBuf, fileLen);
			if (f_res != FR_OK) {
				printf("%-20s SD write error @%s\r\n", "[fileTask.c]", fileBuf);
				printf_fatfs_error(f_res);
				f_close(&tmpFile);
				should_exit = true;
			} else {
				f_sync(&tmpFile); //1K刷新一次
				fnumCount += fnum;
// 				if (i >= 10) {
// 					i = 0;
// #ifdef DEBUG
// 					//分析堆棧使用狀況
// 					if (uxTaskGetStackHighWaterMark(NULL) < stackHighWaterMark) {
// 						stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
// 					}
// #endif
// 				}
			}
			memset(fileBuf, 0, FILE_BUF_SIZE);
			fileLen = 0;
		} else {
			cnt++;
			if (cnt >= 5) {
				should_exit = true;
				printf("%-20s timeout waiting for uart\r\n", "[fileTask.c]");
				// UART_SendString_DMA(&ESP32_USART_PORT, "reset\n");
				HAL_UART_Transmit(&ESP32_USART_PORT, (uint8_t *) "reset\n", strlen("reset\n"), HAL_MAX_DELAY);
			}
		}

		if (should_exit == true || delete == true) {
			//完成 SHA256 計算
			uint8_t hash_output[SHA256_BLOCK_SIZE];
			sha256_final(&sha256_ctx, hash_output);
			// 轉換為十六進位字串
			for (int j = 0; j < SHA256_BLOCK_SIZE; j++) {
				sprintf(hashVal + (j * 2), "%02x", hash_output[j]);
			}
			hashVal[SHA256_BLOCK_SIZE * 2] = '\0';
			f_close(&tmpFile);
			delete = false;
			printf("%-20s fnumCount: %d\r\n", "[fileTask.c]", fnumCount);
			// printf("%-20s minimum stack size: %u\r\n", "[fileTask.c]", stackHighWaterMark);
			uint32_t tmp = xTaskGetTickCount() - timer;
			printf("%-20s total time: %dms\r\n",
			       "[fileTask.c]",
			       tmp);
			deleteTask();
		}
		// vTaskDelay(pdMS_TO_TICKS(1));
	} // while
} // task

static void deleteTask() {
	if (fileSemaphore != NULL) {
		vSemaphoreDelete(fileSemaphore);
		fileSemaphore = NULL;
	}
	if (gcodeRxTaskHandle != NULL) {
		vTaskDelete(gcodeRxTaskHandle);
	}
	// } else if (hashHandle != NULL) {
	// 	cmox_hash_cleanup(hashHandle);
	// 	hashHandle = NULL;
	// }
}

void calFileHash(const char* _filename) {
	FIL tmpFile;
	FRESULT f_res;
	SHA256_CTX sha256_ctx;
	char gcode_line[256] = {0};
	uint8_t hash_output[SHA256_BLOCK_SIZE];

	if (strlen(_filename) <= 0) {
		printf("%-20s no file selected\r\n", "[printerController.c]");
		return;
	}

	f_res = f_open(&tmpFile, _filename, FA_READ);
	if (f_res != FR_OK) {
		printf("%-20s Failed to open file: %s\r\n", "[printerController.c]", _filename);
		return;
	}
	if (f_size(&tmpFile) <= 0) {
		printf("%-20s file has no content\r\n", "[printerController.c]");
		return;
	}
	sha256_init(&sha256_ctx);

	while (f_gets(gcode_line, sizeof(gcode_line), &tmpFile) != NULL) {
		sha256_update(&sha256_ctx, gcode_line, strlen(gcode_line));
	}
	sha256_final(&sha256_ctx, hash_output);

	// 轉換為十六進位字串
	for (int j = 0; j < SHA256_BLOCK_SIZE; j++) {
		sprintf(hashVal + (j * 2), "%02x", hash_output[j]);
	}
	hashVal[SHA256_BLOCK_SIZE * 2] = '\0';
}