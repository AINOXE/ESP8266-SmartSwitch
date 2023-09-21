#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__
#include<ESP8266WebServer.h>
#include"base_sys.h"
/*
Inside "ESP8266WebServer.h":

#define HTTP_MAX_DATA_WAIT 5000000 //ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT 5000000 //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000000 //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000000 //ms to wait for the client to close the connection

I believe that the current version only maintains connection for 2 seconds, which is certainly insufficient for me. After changing these definitions values, it began to wait somewhat longer, which resolved my issue.
*/
extern ESP8266WebServer WebServer;
void WebServer_Init();

extern const char *CONTENT_TYPE_TEXT;
extern const char *CONTENT_TYPE_HTML;
extern const char *CONTENT_TYPE_JSON;

extern const char *WEB_RESULT_SUCCESS_JSON;
extern const char *WEB_RESULT_ERROR_TEMPLATE_JSON;

#define WebServerCheckAuth()                                                                                         \
    if (!WebServer.authenticate((const char *)SystemConfig["admin_account"], (const char *)SystemConfig["admin_password"])) \
        return WebServer.requestAuthentication();
#endif