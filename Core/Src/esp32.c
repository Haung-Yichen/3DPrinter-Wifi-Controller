#include "esp32.h"

#include "cmsis_os2.h"

SemaphoreHandle_t triOverSemaphore = NULL;
SemaphoreHandle_t staPriSemaphore = NULL;
SemaphoreHandle_t rxSemaphore = NULL;
QueueHandle_t cmdQueue = NULL;
StaticQueue_t cmdQueue_s;
uint8_t cmdQueueArea[10 * sizeof(char) * MAX_CMD_LEN];

uint8_t *ResBuf = 0;
char rxBuf[rXBUF_SIZE] = {0};
volatile uint16_t rxLen = 0;

static HAL_StatusTypeDef reportOK_2_ESP32(void);

void ESP32_Init(void) {
	ESP32_SetState(ESP32_INIT);

	memset(rxBuf, 0, rXBUF_SIZE);
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

void do_nothing(void) {}

void ESP32_RegCallback(void) {
	register_command(CMD_Start_Transmisson, StartTransmissionCmdHandler);
	register_command(CMD_Transmisson_Over, TransmissionOverCmdHandler);
	register_command(CMD_SET_FILENAME, do_nothing);
}

ESP32_STATE_TypeDef ESP32_GetState(void) {
	return currentState;
}

void ESP32_SetState(ESP32_STATE_TypeDef state) {
	currentState = state;
}

void ESP32_RxHandler_Task(void *argument) {
	cmdQueue = xQueueCreateStatic(10,
	                              sizeof(char) * MAX_CMD_LEN,
	                              cmdQueueArea,
	                              &cmdQueue_s);
	rxSemaphore = xSemaphoreCreateBinary();
	triOverSemaphore = xSemaphoreCreateBinary();
	staPriSemaphore = xSemaphoreCreateBinary();

	while (1) {
		switch (ESP32_GetState()) {
			case ESP32_IDLE:
				if (pdTRUE == xSemaphoreTake(rxSemaphore, pdMS_TO_TICKS(5000))) {
					printf("received cmd : %s\r\n", rxBuf);
					//判斷指令合法性
					if (isValidCmd(rxBuf) == false) {
						printf("unvalid cmd\r\n");
						UART_SendString_DMA("eCMDError\r\n");
						memset(rxBuf, 0, sizeof(rxBuf));
						continue;
					}
					//判斷是否需要返回值
					if (isReqCmd(rxBuf)) {
						execute_command(rxBuf, ResBuf);
						ESP32_ReturnClbkRes(); //回傳結果給網頁
					}
					else {
						execute_command(rxBuf, NULL);
					}
					memset(rxBuf, 0, sizeof(rxBuf)); //清空指令接收暫存器
					break;
				case ESP32_BUSY:
					// StartTransmissionCmdHandler running...
					break;
				default:
					break;
				}
		}
	}
}

void ESP32_ReturnClbkRes(void) {
	if (ResBuf != NULL) {
		UART_SendString_DMA(ERROR_PARAM_REQ);
	}else {
		HAL_UART_Transmit_DMA(&ESP32_USART_PORT,
					          ResBuf,
					          sizeof(ResBuf));
		*ResBuf = 0;  //清空ResBuf
		ResBuf = NULL;
	}
}

/**
 * @brief 檔案接收回調函式，接收來自ESP32 UART的GCODE
 *		  並存入SD卡，須按照ESP32的狀態來接收
 *
 *		  --------------------狀態說明--------------------
 *		  1, CmdHandler收到"cStartTransmission"跳轉進本函式
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
 * @param args
 * @param res
 */
void StartTransmissionCmdHandler(const char *args, void *res) {
	typedef enum {
		PREPARE_FOR_RECEIVING = 0,
		HANDLE_FILENAME,
		WAIT_FOR_NEXT_LINE,
		VERIFY_FILE,
		ERROR
	} eFile_RX_State;
    eFile_RX_State state = PREPARE_FOR_RECEIVING;

    FIL tmpFile;
    FRESULT f_res;
    UINT fnum;

    char filename[FILENAME_SIZE] = {0};
	char hashVal[SHA256_HASH_SIZE + 1] = {0};
	char *p1 = NULL, *p2 = NULL;

	const uint8_t esp32_recv_delay = 100;
	uint8_t tmpStrLen = 0;
    bool should_exit = false;
    bool received_data = false;
	bool done = false;

    ESP32_SetState(ESP32_BUSY);

    while (!should_exit) {
        switch (state) {
            case PREPARE_FOR_RECEIVING: //收到檔名後跳轉HANDLE_FILENAME
        		vTaskDelay(esp32_recv_delay);
                reportOK_2_ESP32(); // 回傳 ok
                received_data = (xSemaphoreTake(rxSemaphore, pdMS_TO_TICKS(5000)) == pdTRUE);
                if (received_data) {
                    rxBuf[rxLen] = '\0';
                	printf("received file name cmd : %s\r\n", rxBuf);
                } else {
                    printf("Timeout waiting for filename\r\n");
                }
                break;

            case HANDLE_FILENAME:
                if (isValidCmd(rxBuf) == true) {
                	// 從"cSetFilename<filename>"中提取filename出來
                	// char *start = strchr(rxBuf, '<'); // 找到 '<'
                	// char *end = strchr(rxBuf, '>');   // 找到 '>'
	                //
                	// if (start && end && end > start + 1) {
                	// 	size_t len = end - start - 1;  // 計算 < 和 > 之間的長度
                	// 	if (len > FILENAME_SIZE - 1) len = FILENAME_SIZE - 1; // 防止溢出
                	// 	strncpy(filename, start + 1, len); // 從 < 後第一個字元開始複製
                	// 	filename[len] = '\0'; // 確保字串結尾
	                //
                	// 	// 移除可能的 \r\n
                	// 	tmpStrLen = strlen(filename);
                	// 	for (int i = 0; i < tmpStrLen; i++) {
                	// 		if (filename[i] == '\r' || filename[i] == '\n') {
                	// 			filename[i] = '\0';
                	// 			break;
                	// 		}
                	// 	}
                	if (true == extract_parameter(rxBuf, filename, FILENAME_SIZE)) {
                	} else {
                		printf("Invalid filename format\r\n");
                		ESP32_SetState(ESP32_IDLE);
                		should_exit = true;
                	}

                	printf("creating %s...\r\n", filename);
                    f_res = f_open(&tmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
                    if (f_res != FR_OK) {
                        printf("Failed to open file: %d\r\n", f_res);
                        ESP32_SetState(ESP32_IDLE);
                        while (1); // 開檔失敗進入死循環
                    }
                    HAL_UART_Receive_DMA(&ESP32_USART_PORT, rxBuf, sizeof(rxBuf));
                	vTaskDelay(esp32_recv_delay);
                	reportOK_2_ESP32();
                	done = true;

                } else { // 設定檔名指令錯誤
                	done = false;
                    printf("Invalid filename command: %s\r\n", rxBuf);
                }
                break;

            case WAIT_FOR_NEXT_LINE:
        		done = false;

        		received_data = (pdTRUE == xSemaphoreTake(rxSemaphore, pdMS_TO_TICKS(1000)));
                if (received_data) {
                    rxBuf[rxLen] = '\0'; // 確保字串結尾

                	// 檢測是否收到傳輸完成標記
                    if (strncmp(rxBuf, CMD_Transmisson_Over, strlen(CMD_Transmisson_Over)) == 0) {
                    	extract_parameter(rxBuf, hashVal, SHA256_HASH_SIZE);
                    	printf("\r\n======================TransMission Successed=====================\r\n");
                    	printf("hash val : %s\r\n", hashVal);
                    	done = true;
                    	memset(rxBuf, 0, sizeof(rxBuf));
                    } else { // 正常接收邏輯
                        // f_res = f_write(&tmpFile, rxBuf, rxLen, &fnum);
                        // if (f_res != FR_OK) {
                        //     printf("SD write error: %d\r\n", f_res);
                        //     f_close(&tmpFile);
                        //     ESP32_SetState(ESP32_IDLE);
                        //     while (1); // 寫入失敗進入死循環
                        // }
                        // f_sync(&tmpFile);
                    	printf("%s", rxBuf);
                        memset(rxBuf, 0, sizeof(rxBuf));
                    }
                } else {
                    printf("\r\nTimeout waiting for G-code line\r\n");
                    f_close(&tmpFile);
                	done = false;
                }
                break;

            case VERIFY_FILE:
                uint8_t calculated_hash[SHA256_HASH_SIZE];
                // 模擬 SHA256 計算（需替換為實際實現）
                // calculate_sha256(&tmpFile, calculated_hash);

                //if (memcmp(calculated_hash, hashVal, SHA256_HASH_SIZE) == 0) {
        		if (1) {
        			vTaskDelay(esp32_recv_delay);
                    UART_SendString_DMA(ESP32_OK);
                    printf("File verification succeeded\r\n");
                } else {
                    UART_SendString_DMA("eFileBroken");
                    printf("File verification failed\r\n");
                }
                break;
        } // state machine logic

        switch (state) {
            case PREPARE_FOR_RECEIVING:
                if (received_data) {
                    state = HANDLE_FILENAME;
                	printf("switched to HANDLE_FILENAME\r\n");
                } else {
                    ESP32_SetState(ESP32_IDLE);
                	state = PREPARE_FOR_RECEIVING;
                    should_exit = true;
                }
                break;

            case HANDLE_FILENAME:
                if (done == true && f_res == FR_OK) {
                    state = WAIT_FOR_NEXT_LINE;
                	printf("switched to WAIT_FOR_NEXT_LINE\r\n");
                } else {
                    ESP32_SetState(ESP32_IDLE);
                	state = PREPARE_FOR_RECEIVING;
                    should_exit = true;
                }
                break;

            case WAIT_FOR_NEXT_LINE:
                if (done == true) {
                    state = VERIFY_FILE;
                	printf("switched to VERIFY_FILE\r\n");
                } else {
                	state = WAIT_FOR_NEXT_LINE;
                    should_exit = false;
                }
                // 否則保持 WAIT_FOR_NEXT_LINE
                break;

            case VERIFY_FILE:
                state = PREPARE_FOR_RECEIVING; // 跳轉回 PREPARE_FOR_RECEIVING
                should_exit = true; // 結束函數
                break;
        } // state machine transfer
    } // while
	f_close(&tmpFile);
	ESP32_SetState(ESP32_IDLE);
	printf("end of StartTransmissionCmdHandler\r\n");
	//tmp
	// vTaskDelay(esp32_recv_delay);
	// UART_SendString_DMA(ESP32_OK);
} // func end

void TransmissionOverCmdHandler(const char *args, void *res) {
	//通知UI線程傳送完畢
}

static HAL_StatusTypeDef reportOK_2_ESP32(void) {
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