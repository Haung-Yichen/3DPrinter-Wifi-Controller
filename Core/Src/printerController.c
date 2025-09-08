#include "printerController.h"


void PC_init(void) {
	PC_RegCallback();
}

void PC_RegCallback(void) {
	register_command(CMD_Start_To_Print, StartToPrintCmdHandler);
	register_command(CMD_Pause_Printing, PausePrintingCmdHandler);
	register_command(CMD_Stop_printing, StopPrintingCmdHandler);
	register_command(CMD_Go_Home, GoHomeCmdHandler);
	register_command(CMD_Get_Remainning_time, GetRemainingTimeCmdHandler);
	register_command(CMD_Get_Progress, GetProgressCmdHandler);
	register_command(CMD_Get_Nozzle_Temp, GetNozzleTempCmdHandler);
	register_command(CMD_Get_Bed_Temp, GetBedTempCmdHandler);
	register_command(CMD_Set_Nozzle_Temp, SetNozzleTempCmdHandler);
	register_command(CMD_Set_Bed_Temp, SetBedTempCmdHandler);
	register_command(CMD_GetFilament_Weight, GetFilamentWeightCmdHandler);
	register_command(CMD_Emergency_Stop, EmergencyStopCmdHandler);
}

void StartToPrintCmdHandler(const char *args, void *res) {
#ifdef DEBUG
	printf("Start Printing...\r\n");
#endif
	FIL file;
	FRESULT f_res;
	char tmpChar[20]; //佔存每行指令


 
}

void PausePrintingCmdHandler(const char *args, void *res) {
	//通知印表機控制器暫停發送gcode
}

void StopPrintingCmdHandler(const char *args, void *res) {
	//清空列印計數器，印表機回原點
}

void GoHomeCmdHandler(const char *args, void *res) {
	// TODO: 實作回到原點邏輯
}

void GetRemainingTimeCmdHandler(const char *args, void *res) {
	// TODO: 實作回傳剩餘列印時間邏輯
}

void GetProgressCmdHandler(const char *args, void *res) {
	// TODO: 實作回傳列印進度邏輯
}

void GetNozzleTempCmdHandler(const char *args, void *res) {
	// TODO: 實作回傳噴嘴溫度邏輯
}

void GetBedTempCmdHandler(const char *args, void *res) {
	// TODO: 實作回傳熱床溫度邏輯
}

void SetNozzleTempCmdHandler(const char *args, void *res) {
	// TODO: 實作設置噴嘴溫度邏輯
}

void SetBedTempCmdHandler(const char *args, void *res) {
	// TODO: 實作設置熱床溫度邏輯
}

void GetFilamentWeightCmdHandler(const char *args, void *res) {
	// TODO: 實作回傳耗材重量邏輯
}

void EmergencyStopCmdHandler(const char *args, void *res) {
	// TODO: 實作緊急停止邏輯
}
