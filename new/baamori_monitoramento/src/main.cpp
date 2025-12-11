//  Bibliotecas utilizadas
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

// Include class
#include "mpu6050_registers.h"
#include "MPU6050.h"
#include "dataSample.h"
#include "buffer.h"
#include "dataStatistics.h"
#include "decisionTree.h"

#define I2C_PORT I2C_NUM_0
#define SDA_PIN 9
#define SCL_PIN 10

// Tags
static const char *TAG_WIFI = "WIFI";
static const char *TAG_TIME = "TIME";
static const char *TAG_I2C  = "I2C_MPU";

// =============================================================
//                Conectar ao WIFI (STA)
// =============================================================
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

// SNTP INIT
void init_sntp() {
    ESP_LOGI(TAG_TIME, "Iniciando SNTP...");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

extern "C" void app_main()
{   
    // SETUP
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    // Conectar ao WiFi
    wifi_start();

    // Sincronizar horário
    init_sntp();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    time_t now;
    struct tm timeinfo = {0};

    while (timeinfo.tm_year < (2016 - 1900)) {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGW(TAG_TIME, "Aguardando sincronização do tempo...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG_TIME, "Tempo sincronizado!");
    setenv("TZ", "BRT3", 1);  // UTC-3
    tzset();

    // Inicializar I2C + MPU6050
    ESP_LOGI(TAG_I2C, "Configurando I2C...");

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.scl_io_num = SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

    MPU6050 mpu(I2C_PORT, MPU6050_ADDRESS);
    mpu.begin();

    Buffer buffer(2000);
    DataStatistics stats;
    decisionTree tree;

    ESP_LOGI(TAG_I2C, "Entrando no loop...");

    // LOOP 
    while (true)
    {
        if (mpu.update() == ESP_OK)
        {
            dataSample sample(
                mpu.getAccX(), mpu.getAccY(), mpu.getAccZ(),
                mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());

            buffer.push(sample);

            stats.update(
                sample.getAccX(), sample.getAccY(), sample.getAccZ(),
                sample.getGyroX(), sample.getGyroY(), sample.getGyroZ());

            if (buffer.isFull())
            {
                tree.checkFall(stats);

                if (tree.getIsFall()) {
                    ESP_LOGE("FALL", "QUEDA DETECTADA!");

                    time(&now);
                    localtime_r(&now, &timeinfo);

                    char bufferTime[64];
                    strftime(bufferTime, sizeof(bufferTime),
                            "%d/%m/%Y %H:%M:%S", &timeinfo);

                    printf("Data e hora atual: %s\n", bufferTime);
                }

                stats.reset();
                tree.resetFall();
            }else{
                ESP_LOGE("Buffer", "enchendo...");

            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
