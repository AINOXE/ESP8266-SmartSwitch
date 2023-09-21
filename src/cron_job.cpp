#include "base_sys.h"
#include "periphera_devices.h"


void CronJobsHandle()
{
    RtcDateTime now = Rtc.GetDateTime();
    uint32_t nowEpochTime = now.Epoch32Time();
    JsonArray cron_jobs = SystemConfig["cron_jobs"];
    int jobs_size = cron_jobs.size();
    // Serial.printf("定时任务处理: %d个\n",jobs_size);
    for (int i = 0; i < jobs_size; i++)
    {
        JsonObject job = cron_jobs[i];
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
                SwitchControl(job["target"], job["value"]);
            }
            else
            {
                job["exec"] = 0;
            }
        }
        /* 多次定时的任务*/
        else
        {
            int weekAndTime = timeStr.toInt();
            // 70000
            uint8_t timeA_WeekDay = weekAndTime / 10000 % 10;
            // Serial.printf("定时任务处理: WeekDay=%d NowWeekDay=%d\n",timeA_WeekDay,now.DayOfWeek());
            /* 今天不是任务触发天 */
            if (timeA_WeekDay != now.DayOfWeek() && timeA_WeekDay != 0)
            {
                job["exec"] = 0;
                continue;
            }
            int nowTime = now.Hour() * 100 + now.Minute();
            int time=(weekAndTime % 10000);
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
            SwitchControl(job["target"], job["value"]);
        }
    }
}