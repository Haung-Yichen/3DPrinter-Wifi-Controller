#include "ff_print_err.h"

void printf_fatfs_error(FRESULT fresult) {
	switch (fresult) {
		case FR_OK:
			printf("》操作成功。\r\n");
			break;
		case FR_DISK_ERR:
			printf("！！硬體輸入輸出驅動錯誤。\r\n");
			break;
		case FR_INT_ERR:
			printf("！！斷言錯誤。\r\n");
			break;
		case FR_NOT_READY:
			printf("！！實體設備無法運作。\r\n");
			break;
		case FR_NO_FILE:
			printf("！！找不到檔案。\r\n");
			break;
		case FR_NO_PATH:
			printf("！！找不到路徑。\r\n");
			break;
		case FR_INVALID_NAME:
			printf("！！無效的路徑名稱。\r\n");
			break;
		case FR_DENIED:
		case FR_EXIST:
			printf("！！拒絕存取。\r\n");
			break;
		case FR_INVALID_OBJECT:
			printf("！！無效的檔案或路徑。\r\n");
			break;
		case FR_WRITE_PROTECTED:
			printf("！！裝置為寫入保護狀態。\r\n");
			break;
		case FR_INVALID_DRIVE:
			printf("！！無效的邏輯裝置。\r\n");
			break;
		case FR_NOT_ENABLED:
			printf("！！無效的工作區域。\r\n");
			break;
		case FR_NO_FILESYSTEM:
			printf("！！無效的檔案系統。\r\n");
			break;
		case FR_MKFS_ABORTED:
			printf("！！格式化操作因參數錯誤中止。\r\n");
			break;
		case FR_TIMEOUT:
			printf("！！操作逾時。\r\n");
			break;
		case FR_LOCKED:
			printf("！！檔案被鎖定。\r\n");
			break;
		case FR_NOT_ENOUGH_CORE:
			printf("！！無法取得足夠堆空間支援長檔名。\r\n");
			break;
		case FR_TOO_MANY_OPEN_FILES:
			printf("！！開啟檔案數量過多。\r\n");
			break;
		case FR_INVALID_PARAMETER:
			printf("！！無效的參數。\r\n");
			break;
	}
}