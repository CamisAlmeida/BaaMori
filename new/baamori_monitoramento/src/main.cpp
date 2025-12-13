//  Bibliotecas utilizadas
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <inttypes.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
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
#include "timeStamp.h"
#include "Queda.h"

// ThingsBoard
static const char *TB_TOKEN = "Your_token";
static const char *TB_URL = "http://thingsboard.cloud/api/v1/";

#include "esp_http_client.h"

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

//I2C
#define I2C_PORT I2C_NUM_0
#define SDA_PIN 9
#define SCL_PIN 10

// Tags
static const char *TAG_WIFI = "WIFI";
static const char *TAG_TIME = "TIME";
static const char *TAG_I2C  = "I2C_MPU";

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

extern "C" void app_main()
{   
    // SETUP
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    // Conectar ao WiFi
    wifi_start();

    // Sincronizar horário
    init_sntp(TAG_TIME);
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

    Buffer buffer(4000);
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

        tree.checkFall(stats);

        if (tree.getIsFall())
        {
            ESP_LOGE("FALL", "QUEDA DETECTADA!");

            Queda q(stats);

            const DataStatistics& s = q.getStatistics();

            char json[512];
    
            snprintf(json, sizeof(json),
                "{"
                "\"queda\":1,"
                "\"timestamp\":\"%s\","
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
                "}",
                q.getTimestampString().c_str(),
                (unsigned long)s.getCount(),
                s.getMinAX(), s.getMinAY(), s.getMinAZ(),
                s.getMaxAX(), s.getMaxAY(), s.getMaxAZ(),
                s.meanAX(), s.meanAY(), s.meanAZ(),
                s.getMinGX(), s.getMinGY(), s.getMinGZ(),
                s.getMaxGX(), s.getMaxGY(), s.getMaxGZ(),
                s.meanGX(), s.meanGY(), s.meanGZ()
            );
            
            ESP_LOGI("TB_JSON", "Payload: %s", json);

            send_to_thingsboard(json);

            stats.reset();
            tree.resetFall();
        }

        }
    }

}

