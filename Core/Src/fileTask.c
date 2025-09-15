#include "cmsis_os.h"
#include <fileTask.h>

osThreadId_t gcodeRxTaskHandle = NULL;
SemaphoreHandle_t fileSemaphore = NULL;

const osThreadAttr_t gcodeTask_attributes = {
	.name = "Gcode_Rx_Task",
	.stack_size = 128 * 45,
	.priority = (osPriority)osPriorityHigh5,
};

char hashVal[SHA256_HASH_SIZE] = {0};
char fileBuf[FILE_BUF_SIZE] = {0};
char filename[FILENAME_SIZE] = {0};
volatile uint16_t fileLen = 0;
volatile bool delete = false;

/**
 * @brief 刪除任務 並自動刪除指針
 */
static void deleteTask();

void Gcode_RxHandler_Task(void *argument) {
	FIL tmpFile;
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
	bool done = false;


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
				printf("%-20s SD write error\r\n", "[fileTask.c]");
				printf_fatfs_error(f_res);
				f_close(&tmpFile);
				should_exit = true;
			}
			fnumCount += fnum;
			f_sync(&tmpFile); //1K刷新一次
			if (i >= 10) {
				i = 0;
#ifdef DEBUG
				//分析堆棧使用狀況
				if (uxTaskGetStackHighWaterMark(NULL) < stackHighWaterMark) {
					stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
				}
#endif
			}
			// printf("%s", fileBuf);
			memset(fileBuf, 0, FILE_BUF_SIZE);
			fileLen = 0;
		} else {
			cnt ++;
			if (cnt >= 5) {
				should_exit = true;
				printf("%-20s timeout waiting for uart\r\n", "[fileTask.c]");
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
			printf("%-20s minimum stack size: %u\r\n", "[fileTask.c]", stackHighWaterMark);
			printf("%-20s total time: %dms\r\n",
				   "[fileTask.c]",
				   xTaskGetTickCount() - timer);
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
	// } else if (hashHandle != NULL) {
	// 	cmox_hash_cleanup(hashHandle);
	// 	hashHandle = NULL;
	// }
	vTaskDelete(NULL);
}