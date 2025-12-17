//  Bibliotecas utilizadas
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <inttypes.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "sendFall.h"

// Include class
#include "mpu6050_registers.h"
#include "MPU6050.h"
#include "dataSample.h"
#include "buffer.h"
#include "dataStatistics.h"
#include "decisionTree.h"
#include "timeStamp.h"
#include "fall_id.h"
#include "thingsboard.h"
#include "wifiConnect.h"
#include "fall_observer.h"
#include "fall_subject.h"
#include "tb_observer.h"
#include "gateway_observer.h"

#define STATUS_INTERVAL_SEC   (60 * 10)

void get_device_id(char *out, size_t len)
{
    uint64_t mac;
    esp_efuse_mac_get_default((uint8_t *)&mac);

    uint16_t high = (uint16_t)(mac >> 32);
    uint32_t low  = (uint32_t)mac;

    snprintf(out, len, "%04" PRIX16 "%08" PRIX32,
             high, low);
}

//I2C
#define I2C_PORT I2C_NUM_0
#define SDA_PIN 9
#define SCL_PIN 10

// Tags
static const char *TAG_TIME = "TIME";
static const char *TAG_I2C  = "I2C_MPU";

extern "C" void app_main()
{   
    // SETUP
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    // pegar endereço mac
    char device_id[20];
    get_device_id(device_id, sizeof(device_id));

    ESP_LOGI("DEVICE", "Device ID (MAC) = %s", device_id);


    // Conectar ao WiFi
    wifi_start();

    // pegar id da queda
    uint32_t fall_id = load_fall_id();
    ESP_LOGI("NVS", "Último fall_id = %lu", fall_id);


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
    DataStatistics& stats = DataStatistics::getInstance();
    decisionTree& tree = decisionTree::getInstance();

    //Criando observers
    FallSubject fallSubject;

    ThingsBoardObserver tbObserver;
    GatewayObserver gatewayObserver;

    fallSubject.attach(&tbObserver);
    fallSubject.attach(&gatewayObserver);

    ESP_LOGI(TAG_I2C, "Entrando no loop...");
    int64_t last_ok_send = esp_timer_get_time(); 

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

                fall_id++;
                save_fall_id(fall_id);

                fallSubject.notify(stats, fall_id, 1, device_id);

                stats.reset();
                tree.resetFall();
                last_ok_send = esp_timer_get_time();
            }

            int64_t now_us = esp_timer_get_time();

            if ((now_us - last_ok_send) >= STATUS_INTERVAL_SEC * 1000000LL)
            {
                ESP_LOGI("STATUS", "Sem queda - enviando OK");

                fallSubject.notify(stats, fall_id, 0, device_id);

                stats.reset();
                last_ok_send = now_us;
            }

        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }


}

