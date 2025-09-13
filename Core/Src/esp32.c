#include "esp32.h"

#include "cmsis_os2.h"

SemaphoreHandle_t cmdSemaphore = NULL;
SemaphoreHandle_t recvSemaphore = NULL;

// QueueHandle_t cmdQueue = NULL;
// StaticQueue_t cmdQueue_s;
// uint8_t cmdQueueArea[10 * sizeof(char) * MAX_CMD_LEN];

char rxBuf[RXBUF_SIZE] = {0};
volatile uint16_t rxLen = 0;
char cmdBuf[CMD_BUF_SIZE] = {0};

void ESP32_Init(void) {
	ESP32_SetState(ESP32_INIT);

	memset(rxBuf, 0, RXBUF_SIZE);
	HAL_UART_Receive_DMA(&ESP32_USART_PORT, rxBuf, sizeof(rxBuf));
	__HAL_UART_ENABLE_IT(&ESP32_USART_PORT, UART_IT_IDLE);
	ESP32_RegCallback();

	//等esp32傳送ok
	// printf("%-20s waiting for ready");
	// while (1 == strcmp((const char*)"ready", CRxBuf)) {
	// 	printf("%-20s .");
	// }
	ESP32_SetState(ESP32_IDLE);
}

void ESP32_RegCallback(void) {
	register_command(CMD_WIFI_STATUS, WifiStatusHandler);
	register_command(CMD_Start_Transmisson, StartTransmissionHandler);
	register_command(CMD_Transmisson_Over, TransmissionOverHandler);
	register_command(CMD_SET_FILENAME, SetFileNameHandler);
}

ESP32_STATE_TypeDef ESP32_GetState(void) {
	return currentState;
}

void ESP32_SetState(ESP32_STATE_TypeDef state) {
	currentState = state;
}

void ESP32_RxHandler_Task(void *argument) {
	// cmdQueue = xQueueCreateStatic(10,
	//                               sizeof(char) * MAX_CMD_LEN,
	//                               cmdQueueArea,
	//                               &cmdQueue_s);
	cmdSemaphore = xSemaphoreCreateBinary();

	while (1) {
		if (pdTRUE == xSemaphoreTake(cmdSemaphore, pdMS_TO_TICKS(5000))) {
			// printf("%-20s %-30s %s\r\n", "[esp32.c]", "received cmd :", cmdBuf);
			cmdBuf[CMD_BUF_SIZE - 1] = '\0';

			if (isReqCmd(cmdBuf) == true) {
				char ResBuf[10]; //傳回結果緩衝區
				execute_command(cmdBuf, ResBuf);
				UART_SendString_DMA(ResBuf);
				memset(ResBuf, 0, 10);
			} else {
				execute_command(cmdBuf, NULL);
			}
			rxLen = 0;
			memset(cmdBuf, 0, sizeof(cmdBuf));
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void WifiStatusHandler(const char *args, ResStruct_t* _resStruct) {
	char wifiStatus[20] = {0};
	char ip[20] = {0};

	extract_parameter(args, wifiStatus, 20);
	if (wifiStatus[0] == '1') {
		strncpy(ip, wifiStatus+1, 20);
		printf("%-20s Wifi connected @ %s\r\n", "[esp32.c]", ip);;
	} else {
		printf("%-20s Wifi disconnected\r\n", "[esp32.c]");;
	}
}

void StartTransmissionHandler(const char *args, ResStruct_t* _resStruct) {
	ESP32_SetState(ESP32_BUSY);
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();
}

void SetFileNameHandler(const char *args, ResStruct_t* _resStruct) {
	char filename[FILENAME_SIZE] = {0};

	filename[FILENAME_SIZE - 1] = '\0';
	if (false == extract_parameter(args, filename, FILENAME_SIZE)) {
		ESP32_SetState(ESP32_IDLE);
		printf("%-20s Invalid filename format\r\n", "[esp32.c]");
		return;
	}
	printf("%-20s %-30s %s\r\n", "[esp32.c]", "received file name :", filename);

	printf("%-20s %-30s free heap : %d bytes \r\n",
	       "[fileTask.c]",
	       "ready to creat Gcode task",
	       xPortGetFreeHeapSize());

	gcodeRxTaskHandle = osThreadNew(Gcode_RxHandler_Task, filename, &gcodeTask_attributes);
	if (gcodeRxTaskHandle == NULL) {
		ESP32_SetState(ESP32_IDLE);
		printf("%-20s Error creating gcode task\r\n", "[esp32.c]");
		return;
	}
}

void TransmissionOverHandler(const char *args, ResStruct_t* _resStruct) {
	if (gcodeRxTaskHandle == NULL) {
		printf("%-20s gcodeRxTaskHandle is NULL\r\n", "[esp32.c]");
		ESP32_SetState(ESP32_IDLE);
		reportOK_2_ESP32();
		return;
	}

	delete = true;
	xSemaphoreGive(fileSemaphore); // 發送信號量以解除阻塞

	// 等待任務終止（最多 500ms）
	for (int i = 0; i < 200; i++) {
		eTaskState state = eTaskGetState(gcodeRxTaskHandle);
		// printf("%-20s gcodeTaskState: %d\r\n", "[esp32.c]", state);
		if (state == eDeleted) {
			printf("%-20s Gcode task deleted!\r\n", "[esp32.c]");
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
	gcodeRxTaskHandle = NULL;
	vTaskDelay(pdMS_TO_TICKS(100)); // 等待堆記憶體更新
	printf("%-20s free heap: %d bytes\r\n", "[esp32.c]", xPortGetFreeHeapSize());

	extract_parameter(args, hashVal, SHA256_HASH_SIZE);
	printf("%-20s hash val: %s\r\n", "[esp32.c]", hashVal);
	printf("%-20s File verification succeeded\r\n", "[esp32.c]");
	printf("%-20s \r\n======================TransMission Successed=====================\r\n", "[esp32.c]");
	ESP32_SetState(ESP32_IDLE);
	reportOK_2_ESP32();
}

HAL_StatusTypeDef reportOK_2_ESP32(void) {
	if (HAL_UART_GetState(&ESP32_USART_PORT) == HAL_UART_STATE_BUSY_TX) {
		return HAL_BUSY;
	}
	HAL_StatusTypeDef hal_status = HAL_UART_Transmit(&ESP32_USART_PORT,
	                                                 ESP32_OK,
	                                                 strlen(ESP32_OK),
	                                                 HAL_TIMEOUT);
	if (hal_status != HAL_OK) {
		printf("%-20s failed to transmit ok\r\n", "[esp32.c]");
		while (1);
	}
	return hal_status;
}
