#ifndef THINGSBOARD_H
#define THINGSBOARD_H

#include "esp_http_client.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "dataStatistics.h"

// ThingsBoard
static const char *TB_TOKEN = CONFIG_TB_TOKEN;
static const char *TB_URL = "http://thingsboard.cloud/api/v1/";

void send_to_thingsboard(const char *json_payload)
{
    char url[256];
    snprintf(url, sizeof(url), "%s%s/telemetry", TB_URL, TB_TOKEN);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI("THINGSBOARD", "Status = %d",
                 esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE("THINGSBOARD", "Erro ao enviar: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void send_fall(
    const DataStatistics& s,
    uint32_t fall_id,
    int queda_flag,
    const char *device_id
)
{
    // TIMESTAMP REAL (SNTP) 
    
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
        queda_flag,
        (unsigned long)fall_id,
        (unsigned long)s.getCount(),

        s.getMinAX(), s.getMinAY(), s.getMinAZ(),
        s.getMaxAX(), s.getMaxAY(), s.getMaxAZ(),
        s.meanAX(), s.meanAY(), s.meanAZ(),

        s.getMinGX(), s.getMinGY(), s.getMinGZ(),
        s.getMaxGX(), s.getMaxGY(), s.getMaxGZ(),
        s.meanGX(), s.meanGY(), s.meanGZ()
    );

    ESP_LOGI("TB_JSON", "%s", json);
    send_to_thingsboard(json);
}


#endif
