#include "base_sys.h"
#include "periphera_devices.h"

void CronJobsHandle()
{
    RtcDateTime now = Rtc.GetDateTime();
    /* GMT +8 */
    const uint32_t nowEpochTime = now.Epoch32Time() - 28800;
    JsonObject cron_jobs = SystemConfig["cron_jobs"];
    for (JsonPair kv : cron_jobs)
    {
        JsonObject job = kv.value();
        String timeStr = job["time"];
        // Serial.printf("定时任务处理: Time=%s\n",timeStr.c_str());
        int exec = job["exec"];
        /* 一次定时的任务 */
        if (timeStr.length() > 5)
        {
            uint32_t epoch64Time = timeStr.toInt();
            /* 在同一分钟内 触发定时任务 */
            if (nowEpochTime - epoch64Time <= 60)
            {
                /* 已经执行了 就跳过 */
                if (exec)
                    continue;
                job["exec"] = 1;
                SwitchControl(job["target"], job["action"]);
            }
            else
            {
                //Serial.printf("EP:%d  NOWEP:%d\n", epoch64Time, nowEpochTime);
                job["exec"] = 0;
            }
        }
        /* 多次定时的任务*/
        else
        {
            int weekAndTime = timeStr.toInt();
            // 70000
            uint8_t weekDay = weekAndTime / 10000 % 10;
            // Serial.printf("定时任务处理: WeekDay=%d NowWeekDay=%d\n",weekDay,now.DayOfWeek());
            /* 今天不是任务触发天 ！！！周日是0 */
            if (weekDay != now.DayOfWeek() && weekDay != 7)
            {
                job["exec"] = 0;
                continue;
            }
            int nowTime = now.Hour() * 100 + now.Minute();

            int time = (weekAndTime % 10000);
            // Serial.printf("定时任务处理: Time=%d NowTime=%d\n",time,nowTime);
            /*  取出 小时和分钟和 当前小时和分钟 对比 */
            if (time != nowTime)
            {
                job["exec"] = 0;
                continue;
            }

            /* 到达触发时间 被执行过了 跳过 */
            if (exec)
                continue;
            job["exec"] = 1;
            SwitchControl(job["target"], job["action"]);
        }
    }
}