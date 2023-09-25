#include "base_sys.h"
#include "periphera_devices.h"
extern void hardwareTriggerHandle(JsonObject trigger);

void HardwareTriggersHandle()
{
    JsonArray triggers = SystemConfig["triggers"];
    hardwareTriggerHandle(triggers[0]);
    hardwareTriggerHandle(triggers[1]);
    hardwareTriggerHandle(triggers[2]);
}

inline void hardwareTriggerHandle(JsonObject trigger)
{
    String action = trigger["action"];

    if (action == "")
    {
        return;
    }
    int id = trigger["id"];
    int mode = trigger["mode"];
    String name = trigger["name"];
    int target = trigger["target"];

    String false_action = trigger["false_action"];
    /* 获取触发器的电平 */
    int tv = Trigger_ReadHdrValue(id);
    /* 获取上一次触发器的电平 */
    int lastTv = Trigger_GetValue(id);

    bool isTrigExec = false;
    bool isTrueTrig = false;
    switch (mode)
    {
    /* 低电平触发 */
    case 0:
        isTrueTrig = tv == 0;
        isTrigExec = 0 == SwitchControl(target, isTrueTrig ? action : false_action);
        break;
    /* 高电平触发 */
    case 1:
        isTrueTrig = tv == 1;
        isTrigExec = 0 == SwitchControl(target, isTrueTrig ? action : false_action);
        break;
    /* 低电平上升触发 */
    case 2:
        /* 0 -> 1 */
        isTrueTrig = lastTv == 0 && tv == 1;
        if (isTrueTrig)
        {

            isTrigExec = 0 == SwitchControl(target, action);
        }
        else
        {
            isTrigExec = 0 == SwitchControl(target, false_action);
        }

        break;
    /* 高电平下降触发 */
    case 3:
        /* 1 -> 0 */
        isTrueTrig = lastTv == 1 && tv == 0;
        if (isTrueTrig)
        {
            isTrigExec = 0 == SwitchControl(target, action);
        }
        else
        {
            isTrigExec = 0 == SwitchControl(target, false_action);
        }
        break;
    default:
        Serial.println("触发器触发模式错误！");
        break;
    }
    /* 触发执行成功 打印日志 */
    if (isTrigExec)
    {
        Serial.printf("触发器%d 的%s 对%d 执行了 %s\n",
                      id, isTrueTrig ? "成立" : "不成立", target,
                      isTrueTrig ? action.c_str() : false_action.c_str());
    }
    Trigger_SaveValue(id, tv);
}