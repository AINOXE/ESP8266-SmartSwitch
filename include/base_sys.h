#ifndef __BASE_SYS_H__
#define __BASE_SYS_H__

#include <ArduinoJson.h>
/* 系统配置文件锁 */
extern bool SystemConfigLock;
/* 全局配置文件保存路径 */
#define SYSTEM_CONFIG_PATH "/v1-config.json"
/* 全局配置文件版本 */
#define CONFIG_FILE_VERSION 2
/* 默认的全局配置文件 */
#define DEFAULT_SYSYTEM_CONFIG_CONTENT                                                                  \
    "{"                                                                                                 \
    "\"version\": 2,"                                                                                   \
    "\"admin_account\": \"admin\","                                                                     \
    "\"admin_password\": \"02130213\","                                                                 \
    "\"wifi_ssid\": \".inet\","                                                                            \
    "\"wifi_password\": \"123456890\","                                                                  \
    "\"wifi_sta_ip\": null,"                                                                            \
    "\"wifi_sta_mask\": null,"                                                                          \
    "\"wifi_sta_gt\": null,"                                                                            \
    "\"wifi_ap_ssid\": \"GZY-智能开关\","                                                           \
    "\"wifi_ap_password\": \"02130213\","                                                               \
    "\"triggers\": ["                                                                                   \
    "{\"id\":1,\"name\":\"触发器1\",\"mode\":1,\"target\":1,\"action\":null,\"false_action\":null}," \
    "{\"id\":2,\"name\":\"触发器2\",\"mode\":1,\"target\":1,\"action\":null,\"false_action\":null}," \
    "{\"id\":3,\"name\":\"触发器3\",\"mode\":1,\"target\":1,\"action\":null,\"false_action\":null}"  \
    "],"                                                                                                \
    "\"switchs_name\": {"                                                                               \
    "\"l1\":\"通道1\","                                                                               \
    "\"l2\":\"通道2\","                                                                               \
    "\"l3\":\"通道3\","                                                                               \
    "\"l4\":\"通道4\""                                                                                \
    "},"                                                                                                \
    "\"cron_jobs\":{"                                                                                    \
    "   \"测试任务\": {\"time\":42238,\"target\":4,\"action\":\"打开\",\"exec\":0}"          \
    "}"                                                                                                 \
    "}";

/* 全局 Json 配置文件 */
extern DynamicJsonDocument SystemConfig;
/* 配置文件缓存 */
extern char SystemConfigBuf[];

void FileSystem_Init();
void SystemConfig_Init();
void SystemConfig_Save();
void Network_Init();

#define WIFI_CONNECT_TIME_OUT 10000000

#endif