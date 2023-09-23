#include <Arduino.h>
#include <Ticker.h>
#include "web_server.h"
#include "periphera_devices.h"
#include "email_send_handler.h"
#include "oled_display.h"
#include "hdr_trigger.h"
#include "cron_job.h"

Ticker HardwareTriggerTicker;
Ticker CronJobsTicker;

void setup()
{
    /* 串口初始化 */
    Serial.begin(921600);
    /* 文件系统初始化 */
    FileSystem_Init();
    /* 系统配置初始化 */
    SystemConfig_Init();
    /* 外设初始化 */
    PeripheralDevices_Init();
    /* 网络初始化 */
    Network_Init();
    /* Web服务器初始化 */
    WebServer_Init();
    /* 邮件发送处理器初始化 */
    // EmailSendHandler_Init();
    DS1302_SyncNetworkTime();
    /* 设置定时100ms 执行一次触发器检查 */
    HardwareTriggerTicker.attach_ms(100, HardwareTriggersHandle);
    /* 设置定时200ms 执行一次定时任务检查 */
    CronJobsTicker.attach_ms(500, CronJobsHandle);
    
}

void loop()
{
    WebServer.handleClient();
    PeripheralDevices_GetDataLoop();
    delay(1);
}
