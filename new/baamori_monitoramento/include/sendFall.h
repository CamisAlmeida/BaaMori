#ifndef SEND_FALL_H
#define SEND_FALL_H

#include "httpClient.h"
#include "dataStatistics.h"
#include "esp_timer.h"
#include <stdio.h>

void send_fall_to_gateway(DataStatistics &s,
                          uint32_t fall_id,
                          int detected,
                          const char *device_id)
{
    time_t now;
    time(&now);

    // micros desde boot
    int64_t us = esp_timer_get_time();

    // timestamp SNTP em ms + fração em ms
    int64_t ts_ms = ((int64_t)now) * 1000LL + (us % 1000000LL) / 1000LL;

    char json[1024];

    snprintf(json, sizeof(json),
        "{"
        "\"ts\":%" PRId64 ","
        "\"values\":{"
            "\"device_id\":\"%s\","
            "\"queda\":%d,"
            "\"fall_id\":%lu,"
            "\"count\":%lu,"

            "\"acc_min_x\":%.2f,"
            "\"acc_min_y\":%.2f,"
            "\"acc_min_z\":%.2f,"
            "\"acc_max_x\":%.2f,"
            "\"acc_max_y\":%.2f,"
            "\"acc_max_z\":%.2f,"
            "\"acc_mean_x\":%.2f,"
            "\"acc_mean_y\":%.2f,"
            "\"acc_mean_z\":%.2f,"

            "\"gyro_min_x\":%.2f,"
            "\"gyro_min_y\":%.2f,"
            "\"gyro_min_z\":%.2f,"
            "\"gyro_max_x\":%.2f,"
            "\"gyro_max_y\":%.2f,"
            "\"gyro_max_z\":%.2f,"
            "\"gyro_mean_x\":%.2f,"
            "\"gyro_mean_y\":%.2f,"
            "\"gyro_mean_z\":%.2f"
        "}"
        "}",
        ts_ms,
        device_id,
        detected,
        (unsigned long)fall_id,
        (unsigned long)s.getCount(),

        s.getMinAX(), s.getMinAY(), s.getMinAZ(),
        s.getMaxAX(), s.getMaxAY(), s.getMaxAZ(),
        s.meanAX(), s.meanAY(), s.meanAZ(),

        s.getMinGX(), s.getMinGY(), s.getMinGZ(),
        s.getMaxGX(), s.getMaxGY(), s.getMaxGZ(),
        s.meanGX(), s.meanGY(), s.meanGZ()
    );

    char url[128];

    snprintf(url, sizeof(url),
             "http://%s:%d/falls",
             CONFIG_WEB_HOST,
             CONFIG_WEB_PORT);

    http_post(url, json);
}


#endif