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
	// printf("waiting for ready");
	// while (1 == strcmp((const char*)"ready", CRxBuf)) {
	// 	printf(".");
	// }
	ESP32_SetState(ESP32_IDLE);
}

void ESP32_RegCallback(void) {
	register_command(CMD_Start_Transmisson, StartTransmissionCmdHandler);
	register_command(CMD_Transmisson_Over, TransmissionOverCmdHandler);
	register_command(CMD_SET_FILENAME, SetFileNameCmdHandler);
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
	fileSemaphore = xSemaphoreCreateBinary();

	while (1) {
		if (pdTRUE == xSemaphoreTake(cmdSemaphore, pdMS_TO_TICKS(5000))) {
			printf("received cmd : %s\r\n", cmdBuf);
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

void StartTransmissionCmdHandler(const char *args, void *res) {
	ESP32_SetState(ESP32_BUSY);
	vTaskDelay(ESP32_RECV_DELAY);
	reportOK_2_ESP32();
}

void SetFileNameCmdHandler(const char *args, void *res) {
	char filename[FILENAME_SIZE];

	filename[FILENAME_SIZE - 1] = '\0';
	if (false == extract_parameter(args, filename, FILENAME_SIZE)) {
		ESP32_SetState(ESP32_IDLE);
		printf("Invalid filename format\r\n");
	}
	printf("received file name cmd : %s\r\n", filename);

	gcodeRxTaskHandle = osThreadNew(Gcode_RxHandler_Task, NULL, &gcodeTask_attributes);
}

void TransmissionOverCmdHandler(const char *args, void *res) {
	char hashVal[300];

	osThreadTerminate(gcodeRxTaskHandle);
	//目前先跳過驗證
	extract_parameter(args, hashVal, SHA256_HASH_SIZE);
	printf("hash val : %s\r\n", hashVal);
	printf("File verification succeeded\r\n");
	printf("\r\n======================TransMission Successed=====================\r\n");
	//通知UI線程傳送完畢
	ESP32_SetState(ESP32_IDLE);
	vTaskDelay(ESP32_RECV_DELAY);
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
		printf("failed to transmit ok\r\n");
		while (1);
	}
	return hal_status;
}
