#ifndef __EMAIL_SEND_HANDLER_H__
#define __EMAIL_SEND_HANDLER_H__
#include <Arduino.h>
/* 邮件发送间隔 单位*/
#define EMAIL_SEND_INTERVAL 2
#define CALC_EMAIL_SEND_INTERVAL() EMAIL_SEND_INTERVAL*1000000

#define SMTP_HOST "smtp.126.com"
#define SMTP_PORT 25
#define AUTHOR_EMAIL "lzyxedoc@126.com"
#define AUTHOR_PASSWORD "RTOVPEFIMBTKRIZN"

void EmailSendHandler_Init();
void SendEmail(const char *target, const char *subject, const char *content);
#endif