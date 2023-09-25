#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <Arduino.h>
void PeripheralDevices_Init();
void PeripheralDevices_GetDataLoop();
#define RELAY_CLOSE 1
#define RELAY_OPEN 0

#define L_RELAY_CONVERT_VALUE(V) !V

#define Trigger_ReadHdrValue(id) digitalRead(id == 1 ? D0 : id == 2 ? D8 \
                                                                    : A0)
#define Trigger_GetValue(id)                             \
    (id == 1 ? DevicesData.T1 : id == 2 ? DevicesData.T2 \
                                        : DevicesData.T3)
#define Trigger_SaveValue(id, value) \
    if (id == 1)                     \
        DevicesData.T1 = value;      \
    else if (id == 2)                \
        DevicesData.T2 = value;      \
    else if (id == 3)                \
        DevicesData.T3 = value;
#define Switch_GetPin(id) (id == 1 ? D1 : id == 2 ? D2 \
                                      : id == 3   ? D3 \
                                                  : D4);
#define Switch_GetValue(id) (id == 1 ? DevicesData.L1 : id == 2 ? DevicesData.L2 \
                                                    : id == 3   ? DevicesData.L3 \
                                                                : DevicesData.L4)
extern int SwitchControl(int id, String valueStr);
extern int SwitchControl(int id, int value, bool isAppend);

#define DS1302
#ifdef DS1302
#include "RtcDS1302.h"

#define DS1302_CLK D7
#define DS1302_DAT D6
#define DS1302_RST D5

extern RtcDS1302<ThreeWire> Rtc;

extern void DS1302_SyncNetworkTime();
extern void DS1302_PrintDateTime();

#endif

struct _DevicesData_t
{
    int L1;
    int L2;
    int L3;
    int L4;
    int T1;
    int T2;
    int T3;
    int PwmFreq;
    int PwmStep;
};

extern _DevicesData_t DevicesData;

#endif