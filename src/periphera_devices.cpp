#include <Arduino.h>
#include "web_server.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "BH1750.h"
#include "Adafruit_SSD1306.h"
#include "periphera_devices.h"

/*
    @return 取消重复操作 1
            操作成功 0
            操作失败 -1
            无需操作 2
*/
_DevicesData_t DevicesData;
int SwitchControl(int id, String valueStr)
{
    if (valueStr.isEmpty() || valueStr.equals("null"))
    {
        return 2;
    }
    int value = 0;
    bool isAppendV = false;
#pragma region 值转化
    if (id <= 2)
    {
        if (valueStr.startsWith("+"))
        {
            Serial.println("1");
            value = valueStr.substring(1).toInt();
            Serial.println("11");
            isAppendV = true;
        }
        else if (valueStr.startsWith("-"))
        {
            Serial.println("2");
            value = valueStr.toInt();
            Serial.println("22");
            isAppendV = true;
        }
        else if (valueStr.equals("打开"))
        {
            value = 1023;
        }
        else if (valueStr.equals("关闭"))
        {
            value = 0;
        }
        else if (valueStr.equals("反转"))
        {
            value = -1;
        }
        else
        {
            value = valueStr.toInt();
        }
    }
    else
    {
        if (valueStr.equals("打开"))
        {
            value = 1;
        }
        else if (valueStr.equals("关闭"))
        {
            value = 0;
        }
        else if (valueStr.equals("反转"))
        {
            value = -1;
        }
        else
        {
            value = valueStr.toInt();
        }
    }
#pragma endregion

    return SwitchControl(id, value, isAppendV);
}
int SwitchControl(int id, int value, bool isAppend = false)
{

    int pin = Switch_GetPin(id);
    int newValue = value;
    switch (id)
    {
    case 1:
        /* 反转 */
        if (newValue == -1)
        {
            newValue = DevicesData.L1 == 0 ? DevicesData.PwmStep : 0;
            analogWrite(pin, newValue);
            DevicesData.L1 = newValue;
        }
        /* 追加 */
        else if (isAppend)
        {
            newValue = DevicesData.L1 + newValue;

            if (newValue > DevicesData.PwmStep)
                newValue = DevicesData.PwmStep;
            else if (newValue < 0)
                newValue = 0;
            analogWrite(pin, newValue);
            DevicesData.L1 = newValue;
        }
        /* 直接赋值 */
        else
        {
            /* 取消重复操作 */
            if (DevicesData.L1 == newValue)
                return 1;
            analogWrite(pin, newValue);
            DevicesData.L1 = newValue;
        }
        break;
    case 2:
        /* 反转 */
        if (newValue == -1)
        {
            newValue = DevicesData.L2 == 0 ? DevicesData.PwmStep : 0;
            analogWrite(pin, newValue);
            DevicesData.L2 = newValue;
        }
        /* 追加 */
        else if (isAppend)
        {
            newValue = DevicesData.L2 + newValue;
            if (newValue > DevicesData.PwmStep)
                newValue = DevicesData.PwmStep;
            else if (newValue < 0)
                newValue = 0;
            analogWrite(pin, newValue);
            DevicesData.L2 = newValue;
        }
        /* 直接赋值 */
        else
        {
            /* 取消重复操作 */
            if (DevicesData.L2 == newValue)
                return 1;
            analogWrite(pin, newValue);
            DevicesData.L2 = newValue;
        }
        break;
    case 3:
        /* 反转 */
        if (newValue == -1)
            newValue = DevicesData.L3 == 0 ? 1 : 0;
        /* 直接赋值 */
        /* 取消重复操作 */
        else if (DevicesData.L3 == newValue)
            return 1;
        digitalWrite(pin, newValue ? RELAY_OPEN : RELAY_CLOSE);
        DevicesData.L3 = newValue;
        break;
    case 4:
        /* 反转 */
        if (newValue == -1)
            newValue = DevicesData.L4 == 0 ? 1 : 0;
        /* 直接赋值 */
        /* 取消重复操作 */
        else if (DevicesData.L4 == newValue)
            return 1;
        digitalWrite(pin, newValue ? RELAY_OPEN : RELAY_CLOSE);
        DevicesData.L4 = newValue;
        break;

    default:
        Serial.printf("开关控制失败，参数错误! channel:%d value:%d\n", pin, value);
        return -1;
    }
    Serial.printf("开关控制成功: L%d = %d\n", id, newValue);
    return 0;
}

#ifdef DS1302

ThreeWire ThreeWire1(DS1302_DAT, DS1302_CLK, DS1302_RST);
RtcDS1302<ThreeWire> Rtc(ThreeWire1);
#include "NTPClient.h"
#include "WiFiUdp.h"

void DS1302_SyncNetworkTime()
{
    Serial.println("DS1302: 正在同步网络时间...");
    /* 设置ntp服务器 以及时区(北京)*/
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800);
    timeClient.begin();
    /* 更新时间 */
    if (!timeClient.update())
    {
        Serial.println("DS1302: 网络异常，时间同步失败！");
        return;
    }
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    Rtc.SetDateTime(RtcDateTime(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                                ptm->tm_hour, ptm->tm_min, ptm->tm_sec));
    Serial.printf("DS1302: 网络时间同步成功！当前网络时间: %d年%d月%d日 %d:%d:%d\n",
                  ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                  ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void DS1302_PrintDateTime()
{
    char datestring[28];
    RtcDateTime now = Rtc.GetDateTime();
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%04u-%02u-%02u %02u:%02u:%02u"),
               now.Year(),
               now.Month(),
               now.Day(),
               now.Hour(),
               now.Minute(),
               now.Second());
    Serial.println(datestring);
}
#endif

void PeripheralDevices_Init()
{
    DevicesData.L1 = 0;
    DevicesData.L2 = 0;
    DevicesData.L3 = 0;
    DevicesData.L4 = 0;
    DevicesData.T1 = 0;
    DevicesData.T2 = 0;
    DevicesData.T3 = 0;

    DevicesData.PwmFreq = SystemConfig["switchs_config"]["pwm_freq"];
    DevicesData.PwmStep = SystemConfig["switchs_config"]["pwm_step"];

    /* 初始化L1,L2,L3,L4 */
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);

    pinMode(D0, INPUT);
    pinMode(D8, INPUT);
    pinMode(A0, INPUT);
    Serial.printf("PWM初始化中... Freq:%d  Step:%d\n", DevicesData.PwmFreq, DevicesData.PwmStep);

    analogWriteFreq(DevicesData.PwmFreq);
    analogWriteRange(DevicesData.PwmStep);

    /* 复位开关值 */
    analogWrite(D1, 0);
    analogWrite(D2, 0);
    digitalWrite(D3, RELAY_CLOSE);
    digitalWrite(D4, RELAY_CLOSE);

    /**/
#ifdef DS1302
    Rtc.Begin();
#endif
    Serial.println("外设初始化完成!");
}

void PeripheralDevices_GetDataLoop()
{
}