#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include "esp_log.h"
#include "esp_sntp.h"

static inline void init_sntp(const char *tag) {
    ESP_LOGI(tag, "Iniciando SNTP...");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

#endif
