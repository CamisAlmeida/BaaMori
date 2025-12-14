#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H


#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG_WIFI = "WIFI";

// Conectar ao WIFI (STA)
void wifi_start()
{
    ESP_LOGI(TAG_WIFI, "Inicializando WiFi...");

    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, "Camis");
    strcpy((char*)wifi_config.sta.password, "Camis281");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "Conectando ao WiFi...");

    esp_wifi_connect();

    // Esperar conectar
    wifi_ap_record_t info;
    while (true) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
            ESP_LOGI(TAG_WIFI, "WiFi conectado!");
            break;
        }
        ESP_LOGW(TAG_WIFI, "Aguardando conexão WiFi...");
    }
}

#endif