#include <stdio.h>
#include "UITask.h"
#include "FramewinDLG.h"
#include "bsp_xpt2046_lcd.h"
#include "Touch_Calibration.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "task.h"

void touchTask(void *argument) {
	printf("touchTask\n");
	while (1) {
		// printf("X = %d  Y = %d\r\n", GUI_TOUCH_X_MeasureX(),GUI_TOUCH_X_MeasureY());
		GUI_TOUCH_Exec();
		vTaskDelay(10);
	}
}

void GUI_Task(void *argument) {
	while (1) {
		MainTask();
	}

}