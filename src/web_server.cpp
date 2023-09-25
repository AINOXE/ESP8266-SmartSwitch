#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <functional>
#include "web_server.h"
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>

#include "periphera_devices.h"

ESP8266WebServer WebServer(80);

const char *CONTENT_TYPE_TEXT = "text/plan;charset=UTF-8";
const char *CONTENT_TYPE_HTML = "text/html;charset=UTF-8";
const char *CONTENT_TYPE_JSON = "application/json;charset=UTF-8";

const char *WEB_RESULT_SUCCESS_JSON = "{\"code\":0,\"msg\":null}";
const char *WEB_RESULT_ERROR_TEMPLATE_JSON = "{\"code\":%d,\"msg\":\"%s\"}";

extern void WebApi_File_Download();
extern void WebApi_File_Upload();

#define MapUri(URI, METHOD) \
    Serial.printf("MapUri: %s\n", URI); WebServer.on(URI, METHOD, []() -> void

void WebServer_Init()
{
    Serial.println("Web服务器初始化中...");
    /* 主页 */
    MapUri("/", HTTP_GET)
    {
        WebServer.sendHeader("Location", "/main.html.gz");
        WebServer.send(303);
    });
    /* 开关控制 */
    MapUri("/api/switchs/control", HTTP_GET)
    {
        char result_buf[100];
        if (!WebServer.hasArg("id") || !WebServer.hasArg("value"))
        {
            sprintf(result_buf, WEB_RESULT_ERROR_TEMPLATE_JSON, -1, "缺少参数!");
            WebServer.send(200, CONTENT_TYPE_JSON, result_buf);
            return;
        }
        int id = WebServer.arg("id").toInt();
        // Serial.printf("id=%d\n", id);
        /* 通道检查 */
        if (id < 0 || id > 4)
        {
            sprintf(result_buf, WEB_RESULT_ERROR_TEMPLATE_JSON, -1, "通道错误！");
            WebServer.send(200, CONTENT_TYPE_JSON, result_buf);
            return;
        }
        String valueStr = WebServer.arg("value");
        Serial.printf("value=%s\n", valueStr.c_str());

        if (SwitchControl(id, valueStr))
        {
            WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        }
        else
        {
            sprintf(result_buf, WEB_RESULT_ERROR_TEMPLATE_JSON, -1, "操作失败！");
            WebServer.send(200, CONTENT_TYPE_JSON, result_buf);
            return;
        }
    });

    /* 数据获取API */
    MapUri("/api/dev/datas", HTTP_GET)
    {
        char buf[512];
        sprintf(buf,
                "{\"L1\":%d,\"L2\":%d,\"L3\":%d,\"L4\":%d,\"T1\":%d,\"T2\":%d,\"T3\":%d}",
                DevicesData.L1,
                DevicesData.L2,
                DevicesData.L3,
                DevicesData.L4,
                DevicesData.T1,
                DevicesData.T2,
                DevicesData.T3);
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* 触发器获取API*/
    MapUri("/api/triggers/get-config", HTTP_GET)
    {
        char buf[300];
        /* 没有指定查询返回全部 */
        if (!WebServer.hasArg("id"))
        {
            char *largeBuf = (char *)malloc(2048);
            serializeJsonPretty(SystemConfig["triggers"], largeBuf, 2048);
            WebServer.send(200, CONTENT_TYPE_JSON, largeBuf);
            free(largeBuf);
            return;
        }
        int id = WebServer.arg("id").toInt();
        if (id < 0 || id > 3)
        {
            WebServer.send(400);
            return;
        }
        serializeJsonPretty(SystemConfig["triggers"][id - 1], buf, 300);
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* 触发器设置API */
    MapUri("/api/triggers/set-config", HTTP_GET)
    {
        int id = WebServer.arg("id").toInt();
        String name = WebServer.arg("name");
        int mode = WebServer.arg("mode").toInt();
        int target = WebServer.arg("target").toInt();
        String action = WebServer.arg("action");
        action.replace(' ','+');
        String false_action = WebServer.arg("false_action");

        JsonObject trigger = SystemConfig["triggers"][id - 1];
        trigger["name"] = name;
        trigger["mode"] = mode;
        trigger["target"] = target;
        trigger["action"] = action;
        trigger["false_action"] = false_action;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 设置开关配置API */
    MapUri("/api/switchs/set-config", HTTP_GET)
    {
        String l1 = WebServer.arg("l1");
        String l2 = WebServer.arg("l2");
        String l3 = WebServer.arg("l3");
        String l4 = WebServer.arg("l4");

        int pwm_freq = WebServer.arg("pwm_freq").toInt();
        int pwm_step = WebServer.arg("pwm_step").toInt();
        ;

        JsonObject switchs_config = SystemConfig["switchs_config"];
        switchs_config["l1"] = l1;
        switchs_config["l2"] = l2;
        switchs_config["l3"] = l3;
        switchs_config["l4"] = l4;

        switchs_config["pwm_freq"] = pwm_freq;
        switchs_config["pwm_step"] = pwm_step;
        analogWriteFreq(pwm_freq);
        analogWriteRange(pwm_step);

        SystemConfig_Save();
        Serial.printf("开关设置API: \n\tPWM Freq:%d Step:%d\n\tL1=%s\n\tL2=%s\n\tL3=%s\n\tL4=%s\n",
                      pwm_freq, pwm_step,
                      l1.c_str(), l2.c_str(),
                      l3.c_str(), l4.c_str());
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 获取开关配置API */
    MapUri("/api/switchs/get-config", HTTP_GET)
    {
        char buf[1000];
        serializeJsonPretty(SystemConfig["switchs_config"], buf, 1000);
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* 添加定时任务API */
    MapUri("/api/cron-jobs/create", HTTP_GET)
    {
        String name = WebServer.arg("name");

        if (SystemConfig["cron_jobs"].containsKey(name))
        {
            Serial.printf("定时任务-添加: 失败！[%s] 已存在！", name.c_str());
            WebServer.send(200, CONTENT_TYPE_JSON,
                           "{\"code\":-1,\"msg\":\"任务名称已存在！\"}");
            return;
        }
        String time = WebServer.arg("time");
        String target = WebServer.arg("target");
        String action = WebServer.arg("action");
        Serial.printf("定时任务-添加: [%s] Time=%s, Target=%s, Action=%s\n",
                      name.c_str(), time.c_str(), target.c_str(), action.c_str());
        JsonObject cronJob = SystemConfig["cron_jobs"].createNestedObject(name);
        cronJob["time"] = time.toInt();
        cronJob["target"] = target;
        cronJob["action"] = action;
        cronJob["exec"] = 0;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 删除定时任务API */
    MapUri("/api/cron-jobs/delete", HTTP_GET)
    {
        String name = WebServer.arg("name");
        Serial.printf("定时任务-删除: [%s]", name.c_str());
        SystemConfig["cron_jobs"].remove(name);
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });

    /* 更新定时任务API */
    MapUri("/api/cron-jobs/update", HTTP_GET)
    {
        String name = WebServer.arg("name");

        if (!SystemConfig["cron_jobs"].containsKey(name))
        {
            Serial.printf("定时任务-更新: 失败！[%s] 不存在！", name.c_str());
            WebServer.send(200, CONTENT_TYPE_JSON,
                           "{\"code\":-1,\"msg\":\"任务不存在！\"}");
            return;
        }
        String time = WebServer.arg("time");
        String target = WebServer.arg("target");
        String action = WebServer.arg("action");
        Serial.printf("定时任务-更新: [%s] Time=%s, Target=%s, Action=%s\n",
                      name.c_str(), time.c_str(), target.c_str(), action.c_str());
        JsonObject cronJob = SystemConfig["cron_jobs"].createNestedObject(name);
        cronJob["time"] = time.toInt();
        cronJob["target"] = target;
        cronJob["action"] = action;
        cronJob["exec"] = 0;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 获取所有定时任务API */
    MapUri("/api/cron-jobs/get-all", HTTP_GET)
    {
        char *buf = (char *)malloc(2048);
        serializeJsonPretty(SystemConfig["cron_jobs"], buf, 2048);
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
        free(buf);
    });
    /* 安全认证API 认证完成后跳转主页*/
    MapUri("/api/admin/auth", HTTP_GET)
    {

        WebServerCheckAuth();
        WebServer.sendHeader("Location", "/main.html.gz");
        WebServer.send(303);
    });
    /* 系统重启API */
    MapUri("/api/sys/restart", HTTP_GET)
    {
        Serial.println("系统重启API: 重启中...");
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        system_restart();
    });
    /* 系统重置API */
    MapUri("/api/sys/restore", HTTP_GET)
    {
        Serial.println("系统重置API: 重置中...");
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
        LittleFS.remove(SYSTEM_CONFIG_PATH);
        system_restart();
    });
    /* WiFi 连接API */
    MapUri("/api/wifi/connect", HTTP_GET)
    {
        String ssid = WebServer.arg("ssid");
        String password = WebServer.arg("password");
        Serial.printf("WiFi连接API: SSID=%s  密码=%s\n\t连接中", ssid.c_str(), password.c_str());
        WiFi.begin(ssid, password);
        uint32_t wifiConnectStartTime = system_get_time();
        while (WiFi.status() != WL_CONNECTED)
        {
            if (system_get_time() - wifiConnectStartTime > WIFI_CONNECT_TIME_OUT)
            {
                Serial.println("WiFi连接API: 连接超时！");
                WiFi.setAutoConnect(false);
                WiFi.setAutoReconnect(false);
                WebServer.send(200, CONTENT_TYPE_JSON, "{\"code\":-1,\"msg\": \"连接超时！\"}");
                return;
            }
            Serial.print(".");
            delay(500);
        }
        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        Serial.printf("\nWiFi连接API: 连接成功 IP=%s\n", WiFi.localIP().toString().c_str());
        SystemConfig["wifi_ssid"] = ssid;
        SystemConfig["wifi_password"] = password;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    // /* WiFi 连接信息保存 API*/
    // MapUri("/api/wifi/save-con-info", HTTP_GET)
    // {
    //     String ssid = WebServer.arg("ssid");
    //     String password = WebServer.arg("password");
    //     SystemConfig["wifi_ssid"] = ssid;
    //     SystemConfig["wifi_password"] = password;
    //     Serial.printf("保存WiFi连接配置 SSID: %s  Pwd: %s\n", ssid.c_str(), password.c_str());
    //     SystemConfig_Save();
    //     WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    // });
    /* 设置静态IP地址API */
    MapUri("/api/wifi/set-static-ip", HTTP_GET)
    {
        String ip = WebServer.arg("ip");
        String netmask = WebServer.arg("netmask");
        String gateway = WebServer.arg("gateway");
        Serial.printf("设置静态IP: IP=%s  MASK=%s GT=%s\n", ip.c_str(), netmask.c_str(), gateway.c_str());
        SystemConfig["wifi_sta_ip"] = ip;
        SystemConfig["wifi_sta_mask"] = netmask;
        SystemConfig["wifi_sta_gt"] = gateway;
        SystemConfig_Save();
        WebServer.send(200, CONTENT_TYPE_JSON, WEB_RESULT_SUCCESS_JSON);
    });
    /* 获取STA IP地址信息API */
    MapUri("/api/wifi/get-ip-info", HTTP_GET)
    {
        String localIp = WiFi.localIP().toString();
        String netmask = WiFi.subnetMask().toString();
        String gateway = WiFi.gatewayIP().toString();
        Serial.printf("获取IP信息API: IP=%s  MASK=%s  GT=%s\n", localIp.c_str(), netmask.c_str(), gateway.c_str());
        char buf[100];
        ;
        sprintf(buf, "{\"ip\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\"}",
                localIp.c_str(), netmask.c_str(), gateway.c_str());
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /* WiFi扫描API */
    MapUri("/api/wifi/scan", HTTP_GET)
    {
        Serial.println("WiFi扫描API: 开始扫描中...");
        int count = WiFi.scanNetworks();
        char buf[1024];
        char *bufPtr = buf;
        *(bufPtr++) = '[';
        for (int i = 0; i < count; i++)
        {
            bufPtr += sprintf(bufPtr, "{\"ssid\":\"%s\",\"rssi\":%d},",
                              WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            Serial.printf("\t%s   (%d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }

        Serial.printf("WiFi扫描API: 扫描结束，共%d个\n", count);
        if (*(bufPtr - 1) == ',')
            *(bufPtr - 1) = ']';
        else
        {
            *bufPtr = ']';
            *(++bufPtr) = '\0';
        }
        WebServer.send(200, CONTENT_TYPE_JSON, buf);
    });
    /*文件上传页面*/
    MapUri("/fsup", HTTP_GET)
    {
        WebServer.send(200, CONTENT_TYPE_HTML,
                       "<form action=\"/api/fsup\" method=\"POST\" enctype=\"multipart/form-data\">"
                       "<input type=\"file\" name=\"mufile\">"
                       "<input type=\"submit\" name=\"mufile\">"
                       "</form>");
    });
    /*文件上传API*/
    MapUri("/api/fsup", HTTP_POST)
    {
        WebServer.send(200, CONTENT_TYPE_TEXT, "文件上传成功！");
    },WebApi_File_Upload);

    /*文件下载API*/
    WebServer.onNotFound(WebApi_File_Download);

    /*启动服务器*/
    WebServer.begin();
    WebServer.keepAlive(true);
    Serial.println("Web服务器初始化完成...");
}

String getContentTypeByFileName(String filename)
{
    if (filename.endsWith(".html") || filename.endsWith(".html.gz"))
        return "text/html;charset=UTF-8";
    if (filename.endsWith(".css") || filename.endsWith(".css.gz"))
        return "text/css;charset=UTF-8";
    if (filename.endsWith(".js") || filename.endsWith(".js.gz"))
        return "text/javascript;charset=UTF-8";
    return "text/plan;charset=UTF-8";
}
File fsUploadFile;
void WebApi_File_Download()
{
    WebServer.keepAlive(false);
    String uri = WebServer.uri();
    if (!LittleFS.exists(uri))
    {
        Serial.printf("文件下载-404: 文件 %s 不存在\n", uri.c_str());
        WebServer.send(404, CONTENT_TYPE_TEXT, "文件不存在！");
        return;
    }
    File downloadFile = LittleFS.open(uri, "r");
    Serial.printf("文件下载-开始: %s  发送中...\n", uri.c_str());

    size_t sendSize = WebServer.streamFile(downloadFile, getContentTypeByFileName(uri));
    if (sendSize == downloadFile.size())
    {
        Serial.printf("文件下载-成功: %s  大小:%d\n", uri.c_str(), sendSize);
    }
    else
    {
        Serial.printf("文件下载-失败: %s  发送失败！\n", uri.c_str());
    }
    downloadFile.close();
}

void WebApi_File_Upload()
{
    HTTPUpload &upload = WebServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        // 如果上传状态为UPLOAD_FILE_START
        String filename = upload.filename;
        // 建立字符串变量用于存放上传文件名
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        // 为上传文件名前加上 "/"
        Serial.printf("文件上传-开始: %s\n", filename.c_str());
        // 通过串口监视器输出上传文件的名称
        fsUploadFile = LittleFS.open(filename, "w");
        // 在SPIFFS中建立文件用于写入用户上传的文件数据
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        // 如果上传状态为UPLOAD_FILE_WRITE
        if (fsUploadFile)
            // 向SPIFFS文件写入浏览器发来的文件数据
            fsUploadFile.write(upload.buf, upload.currentSize);

        Serial.printf("文件上传-写入: %d 字节\n", upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        // 如果上传状态为UPLOAD_FILE_END
        if (fsUploadFile)
        {
            // 如果文件成功建立
            fsUploadFile.close();
            // 将文件关闭
            Serial.printf("文件上传-完成: 大小: %d 字节\n", upload.totalSize);
            WebServer.send(200);
        }
        else
        {
            // 如果文件未能成功建立
            Serial.println("文件上传-失败: 文件创建失败！");
            // 通过串口监视器输出报错信息
            WebServer.send(500, "text/plain", "500 文件创建失败！");
        }
    }
}