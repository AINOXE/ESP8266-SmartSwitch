#ifdef OLED_DISPLAY
#include <Arduino.h>
#include "oled_display.h"
#include "periphera_devices.h"
#include <ESP8266WiFi.h>

extern void OledDisplay_UpdateNetworkStatus();
extern void OledDisplay_UpdateEnvStatus();
extern void OledDisplay_UpdateDevicesData();

extern const unsigned char img_caixukun[];
void OledDisplay_Init()
{
    OledDisplay.clearDisplay();
    /*设置字体颜色 不写无法使用OledDisplay.print等方法 */
    OledDisplay.setTextColor(WHITE);
    OledDisplay.drawBitmap(0, 0, img_caixukun, 128, 64, 1);
    OledDisplay.display();
}

void OledDisplay_Update()
{
    OledDisplay.clearDisplay();
    OledDisplay_UpdateDevicesData();
    OledDisplay_UpdateEnvStatus();
    OledDisplay_UpdateNetworkStatus();
    OledDisplay.display();
}

extern const unsigned char font_lib16[][32];
extern const unsigned char symbol_lib16[][32];
extern const unsigned char symbol_lib8[][8];

#define OledDisplay_Show16x16(X, Y, CONTENT) \
    OledDisplay.drawBitmap((X - 1) * 16, (Y - 1) * 16, CONTENT, 16, 16, 1);
/* 更新网络显示 */
inline void OledDisplay_UpdateNetworkStatus()
{
    /* 显示WiFi是否连接图标 */
    OledDisplay_Show16x16(1, 1, symbol_lib16[WiFi.status() == WL_CONNECTED ? 2 : 3]);
    // /* 显示热点是否开启 */
    // OledDisplay.setTextSize(1);
    // OledDisplay.setCursor(64, 32);
    // OledDisplay.printf("AP:%d", 1);
    // /* 显示IP地址第四段 */
    // OledDisplay.setCursor(52, 40);
    // OledDisplay.printf("IP:%d", (int)ipaddr_addr(WiFi.localIP().toString().c_str()) >> 24);
}
/* 更新环境状态显示 */
inline void OledDisplay_UpdateEnvStatus()
{
    /* 智能开关 */
    OledDisplay_Show16x16(3, 1, font_lib16[15]);
    OledDisplay_Show16x16(4, 1, font_lib16[16]);
    OledDisplay_Show16x16(5, 1, font_lib16[28]);
    OledDisplay_Show16x16(6, 1, font_lib16[29]);
}
/* 更新外设数据 */
inline void OledDisplay_UpdateDevicesData()
{
    /* 切换字体大小 12x16 */
    OledDisplay.setTextSize(2);
}
#endif