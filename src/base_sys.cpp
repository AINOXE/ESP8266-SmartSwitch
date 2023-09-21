#include<Arduino.h>
/* 文件系统库 */
#include<LittleFS.h>
/* JSON 库 */
#include<ArduinoJson.h>
/* Wifi 库 */
#include<ESP8266WiFi.h>

#include "base_sys.h"

/* 全局 Json 配置文件 */
DynamicJsonDocument SystemConfig(5120);
/* 全局 Json 配置文件Buf */
char SystemConfigBuf[3000] = {0};

/* 默认的全局配置文件 */
const char *DefaultSystemConfigContent = DEFAULT_SYSYTEM_CONFIG_CONTENT;
extern void system_restart();
/* 文件系统初始化 */
void FileSystem_Init()
{
    Serial.println("文件系统初始化中...");
    if (!LittleFS.begin())
    {
        Serial.println("文件系统格式化中...");
        LittleFS.format();
    }
    Serial.println("文件系统初始化成功！");
}
/* 系统配置文件初始化 */
void SystemConfig_Init()
{
    Serial.println("系统配置文件初始化中...");
    if (!LittleFS.exists(SYSTEM_CONFIG_PATH))
    {
        Serial.println("默认系统配置文件文件创建中...");
        File file = LittleFS.open(SYSTEM_CONFIG_PATH, "w");
        file.write(DefaultSystemConfigContent, strlen(DefaultSystemConfigContent));
        file.flush();
        file.close();
    }
    File configFile = LittleFS.open(SYSTEM_CONFIG_PATH, "r");
    size_t fileSize = configFile.size();
    Serial.printf("系统配置文件 大小: %d 字节", fileSize);
    if (configFile.readBytes(SystemConfigBuf, fileSize) != fileSize)
    {
        Serial.println("ERR: 系统配置文件读取失败！");
    }
    Serial.print("系统配置文件读取成功 ->\n");
    Serial.println(SystemConfigBuf);
    Serial.print(deserializeJson(SystemConfig, SystemConfigBuf) == DeserializationError::Code::Ok
                     ? "配置文件 反序列化成功!\n"
                     : "配置文件 反序列化失败!\n");
    int version = SystemConfig["version"].as<int>();
    if (version != CONFIG_FILE_VERSION)
    {
        Serial.println("配置文件更新中...");
        LittleFS.remove(SYSTEM_CONFIG_PATH);
        
        system_restart();
    }
    configFile.close();
}
/* 系统配置文件保存 */
void SystemConfig_Save()
{
    Serial.printf("系统配置文件保存中... 空闲内存: %d\n",ESP.getFreeHeap());
    char *buf = (char *)malloc(5000);
    size_t size = serializeJsonPretty(SystemConfig, buf, 5000);
    File file = LittleFS.open(SYSTEM_CONFIG_PATH, "w");
    file.write((const char *)buf, size);
    file.close();
    free(buf);
    Serial.println("系统配置文件保存成功！");
}

void Network_Init()
{
    Serial.println("\nWiFi初始化中...");
    WiFi.disconnect();
    /* 开启热点 */
    WiFi.mode(WIFI_AP_STA);
    
    IPAddress wifiApLocalIp(192, 168, 213, 1);
    IPAddress wifiApNetmask(255, 255, 255, 0);
    IPAddress wifiApGateway(192, 168, 213, 1);

    WiFi.softAPConfig(wifiApLocalIp, wifiApGateway, wifiApNetmask);
    /* 获取系统配置文件中的Wifi名称和密码 */
    String wifiApSSID = SystemConfig["wifi_ap_ssid"];
    String wifiApPassword = SystemConfig["wifi_ap_password"];
    /* 开启WIFI热点 */
    WiFi.softAP(wifiApSSID, wifiApPassword);

    Serial.printf("WiFi热点已开启!\n\t名称: %s\n\t密码: %s\n\tIP: 192.168.213.1\n", wifiApSSID.c_str(), wifiApPassword.c_str());
    /* 获取系统配置文件中的AP账号和密码，然后进行 WIFI 连接 */
    String wifiSSID = SystemConfig["wifi_ssid"];
    String wifiPassword = SystemConfig["wifi_password"];
    String wifiIpStr = SystemConfig["wifi_sta_ip"];
    String wifiMaskStr = SystemConfig["wifi_sta_mask"];
    String wifiGatewayIpStr = SystemConfig["wifi_sta_gt"];
    /* 静态IP 设置 */
    if(!wifiIpStr.isEmpty() && !wifiGatewayIpStr.isEmpty() 
        && !wifiIpStr.equals("null") && !wifiGatewayIpStr.equals("null"))
    {
        IPAddress wifiIp,wifiGatewayIp,wifiNetmask;
        wifiIp.fromString(wifiIpStr);
        wifiGatewayIp.fromString(wifiGatewayIpStr);
        wifiNetmask.fromString(wifiMaskStr);
        WiFi.config(wifiIp,wifiGatewayIp,wifiApNetmask);
        Serial.printf("静态IP已设置! \n\tIP:%s\n\tMASK:%s\n\tGT:%s\n",
            wifiIpStr.c_str(),
            wifiMaskStr.c_str(),
            wifiGatewayIpStr.c_str());
    }

    Serial.printf("WiFi连接中... \n\t名称: %s \n\t密码: %s\n\t", wifiSSID.c_str(), wifiPassword.c_str());
    /* 发起WIFI连接 */
    WiFi.begin(wifiSSID, wifiPassword);
    /* 获取连接开始时间 */
    uint32_t wifiConnectStartTime = system_get_time();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (system_get_time() - wifiConnectStartTime > WIFI_CONNECT_TIME_OUT)
        {
            WiFi.setAutoConnect(false);
            WiFi.setAutoReconnect(false);
            Serial.println("\nWiFi连接超时!");
            return;
        }
        Serial.print(".");
        delay(500);
    }
    Serial.printf("\nWiFi连接成功 IP: %s\n", WiFi.localIP().toString().c_str());

    /* 连接成功后 设置自动连接和自动重连以防止掉线*/
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
}


