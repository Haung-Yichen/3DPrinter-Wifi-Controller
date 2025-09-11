#ifndef _CMD_HANDLER_H_
#define _CMD_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_CMD_COUNT 20	//最大命令數量
#define MAX_CMD_LEN   20	//單條命令字元數

//建立或執行命令時錯誤種類枚舉
typedef enum CmdHandlerErrStat_Typedef {
	CMD_OK = 0,  //正常
	QTY_OVER,    //超過命令數量限制
	NAME_ERR,    //命令命名錯誤
	CALBCK_ERR,  //回調函數無效
	REPEAT_ERR,  //命令名稱重複
	CMD_ERR,	 //命領無效
	EXC_ERR		 //執行錯誤
} CmdHandlerStat;

#ifdef __cplusplus
extern "C" {
#endif

/*  回調函數原型定義  */
typedef void (*CommandCallback)(const char *args, void *res);

/**
 *@brief 註冊命令
 * @param cmdName    命令名稱 最常不可超過MAX_CMD_LEN
 * @param callback   實際執行命令的callback func
 * @return           命令註冊狀態
 */
CmdHandlerStat register_command(const char *cmdName, CommandCallback callback);

/**
 *@brief 執行命令，放在命令處理線程裡
 *		 解析完命令後呼叫實際callback func執行
 * @param cmd		命令 用空格分割參數
 * @param res		回傳參數的存放變數位址
 * @return			命令執行狀態
 */
CmdHandlerStat execute_command(const char *cmd, void *res);

/**
 * @brief 判斷命令是否需要返回值
 * @param cmd
 * @return
 */
bool isReqCmd(const char *cmd);

/**
 * @brief 判斷是否為合法指令，只檢查命令本身而不檢查參數
 * @param cmd
 * @return CmdHandlerStat
 */
CmdHandlerStat isValidCmd(const char *cmd);

/**
 * 從命令字串中提取 <> 內的參數
 * @param input: 輸入命令字串
 * @param output: 輸出參數緩衝區
 * @param max_len: 緩衝區最大長度
 * @return: true 表示成功，false 表示失敗
 */
bool extract_parameter(const char *input, char *output, size_t max_len);

#ifdef DEBUG
void print_all_cmd(void); //印出所有已註冊命令
#endif

#ifdef __cplusplus
#endif
#endif /* _CMD_HANDLER_H_ */
