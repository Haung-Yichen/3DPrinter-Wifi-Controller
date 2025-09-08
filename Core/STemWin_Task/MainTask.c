#include "bsp_ili9341_lcd.h"
#include "GUI.h"
#include "DIALOG.h"

#define FPS 60
#define FRAME_DELAY_MS (0.01666666667 * 1000)
#define MOVEMENT 6

typedef struct {
    uint16_t TopLeftX;
    uint16_t TopLeftY;
    uint16_t BottomRightX;
    uint16_t BottomRightY;
    uint8_t TextWidth;
    uint8_t TextHeight;
} GUI_RectTypeDef;

typedef enum {
    RIGHT_DOWN = 0,
    RIGHT_UP = 1,
    LEFT_DOWN = 2,
    LEFT_UP = 3,
} DirTypeDef;

const char *pText = "emWin Test";

void TestTask(void) {
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(&GUI_Font24B_1);
    GUI_Clear();

    // 初始化文字区域
    GUI_RectTypeDef TextRect;
    TextRect.TextWidth = GUI_GetStringDistX(pText);
    TextRect.TextHeight = GUI_GetFontSizeY();
    TextRect.TopLeftX = 0;
    TextRect.TopLeftY = 0;
    TextRect.BottomRightX = TextRect.TopLeftX + TextRect.TextWidth;
    TextRect.BottomRightY = TextRect.TopLeftY + TextRect.TextHeight;

    // 创建内存设备
    GUI_MEMDEV_Handle hMem = GUI_MEMDEV_Create(0, 0, TextRect.TextWidth, TextRect.TextHeight);
    if (hMem) {
        // 在内存设备中绘制文字
        GUI_MEMDEV_Select(hMem);
        GUI_Clear();
        GUI_DispStringAt(pText, 0, 0);
        GUI_MEMDEV_Select(0);
    }

    DirTypeDef Dir = RIGHT_DOWN;

    while (1) {
        // 清除旧位置（使用背景色填充）
        GUI_ClearRect(TextRect.TopLeftX, TextRect.TopLeftY, 
                     TextRect.BottomRightX, TextRect.BottomRightY);

        // 更新位置
        switch (Dir) {
            case RIGHT_DOWN:
                TextRect.TopLeftX += MOVEMENT;
                TextRect.TopLeftY += MOVEMENT;
                if (TextRect.BottomRightX >= LCD_GetXSize()) Dir = LEFT_DOWN;
                else if (TextRect.BottomRightY >= LCD_GetYSize()) Dir = RIGHT_UP;
                break;

            case RIGHT_UP:
                TextRect.TopLeftX += MOVEMENT;
                TextRect.TopLeftY -= MOVEMENT;
                if (TextRect.BottomRightX >= LCD_GetXSize()) Dir = LEFT_UP;
                else if (TextRect.TopLeftY <= 0) Dir = RIGHT_DOWN;
                break;

            case LEFT_DOWN:
                TextRect.TopLeftX -= MOVEMENT;
                TextRect.TopLeftY += MOVEMENT;
                if (TextRect.TopLeftX <= 0) Dir = RIGHT_DOWN;
                else if (TextRect.BottomRightY >= LCD_GetYSize()) Dir = LEFT_UP;
                break;

            case LEFT_UP:
                TextRect.TopLeftX -= MOVEMENT;
                TextRect.TopLeftY -= MOVEMENT;
                if (TextRect.TopLeftX <= 0) Dir = RIGHT_UP;
                else if (TextRect.TopLeftY <= 0) Dir = LEFT_DOWN;
                break;
        }

        // 更新右下角坐标
        TextRect.BottomRightX = TextRect.TopLeftX + TextRect.TextWidth;
        TextRect.BottomRightY = TextRect.TopLeftY + TextRect.TextHeight;

        // 使用内存设备绘制新位置
        if (hMem) {
            GUI_MEMDEV_CopyToLCDAt(hMem, TextRect.TopLeftX, TextRect.TopLeftY);
        } else {
            // 如果内存设备创建失败，回退到直接绘制
            GUI_DispStringAt(pText, TextRect.TopLeftX, TextRect.TopLeftY);
        }

        GUI_Exec();
        GUI_Delay(FRAME_DELAY_MS);
    }

    // 释放内存设备（实际不会执行到这里，因为while(1)）
    if (hMem) {
        GUI_MEMDEV_Delete(hMem);
    }
}