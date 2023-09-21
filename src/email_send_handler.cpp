#include "email_send_handler.h"
#include "ESP_Mail_Client.h"
#include "base_sys.h"
#include "HeapStat.h"
HeapStat heapInfo;
SMTPSession smtp;
char content_buf[2048];
unsigned int lastSendTime = 0;
ESP_Mail_Session session;
Session_Config config;
void getSmtpStatusCallback(SMTP_Status status);
void EmailSendHandler_Init()
{
    // /* 开启Debug输出 */
    // smtp.debug(0);
    // /* 注册回调函数，获取邮件发送状态 */
    // smtp.callback(getSmtpStatusCallback);

    smtp.debug(0);

    smtp.callback(getSmtpStatusCallback);

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;

    config.login.user_domain = "";

    /*
    Set the NTP config time
    For times east of the Prime Meridian use 0-12
    For times west of the Prime Meridian add 12 to the offset.
    Ex. American/Denver GMT would be -6. 6 + 12 = 18
    See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
    */
    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;
}
void SendEmail(const char *target, const char *subject, const char *content)
{
    /* 等上一次发完后2s */
    unsigned int readySendTime = lastSendTime + CALC_EMAIL_SEND_INTERVAL();
    Serial.printf("邮件发送 开始: 向 %s 发送主题为 %s 的邮件中...\n", target, subject);
    Serial.printf("邮件发送 等待: ");
    while (system_get_time() < readySendTime)
    {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    /* 定义smtp message消息类 */
    SMTP_Message message;
    /* 定义邮件消息类的名称，发件人，标题和添加收件人 */
    message.sender.name = SystemConfig["wifi_ap_ssid"].as<String>().c_str();
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;
    message.addRecipient("Sara", target);
    /* 设置message html 格式和内容*/
    // 天然气报警通知
    // 当前天然气浓度=165ppm，超过设定值140ppm。现已打开排风扇进行通风，请尽快检查！

    sprintf(content_buf, "<center><h1 style=\"color: red;margin-top: 50px;\">%s</h1><h3>%s</h3></center>",
            subject, content);
    message.html.content = content_buf;
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    /* 连接smtp服务器 */
    if (!smtp.connect(&config))
    {
        Serial.printf("邮件发送 失败: Smtp服务器连接失败, 状态码: %d, 错误码: %d, 原因: %s\n",
                      smtp.statusCode(),
                      smtp.errorCode(),
                      smtp.errorReason().c_str());
        goto exit;
    }

    Serial.printf("邮件发送 进行: Smtp服务连接成功! %s:%d\n", SMTP_HOST, SMTP_PORT);

    // /* 调用发送邮件函数，失败的话，获取失败信息 */
    // if (!MailClient.sendMail(&smtp, &message))
    // {
    //     Serial.println("发送邮件失败，失败原因是");
    //     Serial.println("发送邮件失败，失败原因是 , " + smtp.errorReason());
    // }
    // /* 调用发送邮件函数，失败的话，获取失败信息 */
    if (!MailClient.sendMail(&smtp, &message))
    {
        Serial.printf("邮件发送 失败: 状态码: %d, 错误码: %d, 原因: %s\n",
                      smtp.statusCode(),
                      smtp.errorCode(),
                      smtp.errorReason().c_str());
        message.clear();
        goto exit;
    }
    lastSendTime = system_get_time();

exit:
    heapInfo.collect();
    // heapInfo.print();
}

/* 获取发送状态的回调函数 */
void getSmtpStatusCallback(SMTP_Status status)
{
    /* 输出邮件发送状态信息 */
    // Serial.println(status.info());

    /*状态获取成功，打印状态信息 */
    if (status.success())
    {
        Serial.println("------------邮件发送结果------------");
        ESP_MAIL_PRINTF("邮件发送 完成: 成功%d个, 失败%d个\n", status.completedCount(), status.failedCount());
        struct tm dt;
        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            /* 依次获取发送邮件状态 */
            SMTP_Result result = smtp.sendingResult.getItem(i);
            time_t ts = (time_t)result.timestamp;
            localtime_r(&ts, &dt);
            ESP_MAIL_PRINTF("\n收件人: %s\n", result.recipients.c_str());
            ESP_MAIL_PRINTF("状态: %s\n", result.completed ? "成功" : "失败");
            ESP_MAIL_PRINTF("发送时间: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
            ESP_MAIL_PRINTF("邮件标题: %s\n", result.subject.c_str());
        }

        Serial.println("------------------------------------");
        smtp.sendingResult.clear();
    }
}