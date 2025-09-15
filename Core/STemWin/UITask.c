#include "UITask.h"
#include "FramewinDLG.h"
#include "Touch_Calibration.h"
#include "FreeRTOS.h"
#include "task.h"

void touchTask(void *argument) {
	while (1) {
		GUI_TOUCH_Exec();
		vTaskDelay(10);
	}
}

void GUI_Task(void *argument) {
	while (1) {
		MainTask();
	}

}