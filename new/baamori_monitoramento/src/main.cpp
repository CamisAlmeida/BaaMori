#include <stdio.h>
#include <string.h>
#include <inttypes.h>   // ← NECESSÁRIO para PRIu32
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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

static const char *TAG = "I2C_MPU";

extern "C" void app_main()
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    ESP_LOGI(TAG, "INICIANDO APP_MAIN ");

    // CONFIG I2C
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.scl_io_num = SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    ESP_LOGI(TAG, "Configurando I2C...");
    esp_err_t err;

    err = i2c_param_config(I2C_PORT, &conf);
    ESP_LOGW(TAG, "i2c_param_config -> %s", esp_err_to_name(err));

    err = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    ESP_LOGW(TAG, "i2c_driver_install -> %s", esp_err_to_name(err));

    // MPU6050
    ESP_LOGI(TAG, "Inicializando MPU6050...");

    MPU6050 mpu(I2C_PORT, MPU6050_ADDRESS);

    bool ok = mpu.begin();
    ESP_LOGW(TAG, "mpu.begin -> %d", ok);

    if (!ok)
    {
        ESP_LOGE(TAG, "MPU6050 NÃO DETECTADO!");
    }

    // BUFFER, STATISTICS, TREE
    Buffer buffer(2000);
    DataStatistics stats;
    decisionTree tree;

    ESP_LOGI(TAG, "Entrando no loop...");

    while (true)
    {
        esp_err_t ret = mpu.update();

        if (ret == ESP_OK)
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
                    ESP_LOGE(TAG, "QUEDA DETECTADA!");
                } else {
                    ESP_LOGI(TAG, "Nenhuma queda");
                }

                stats.reset();
                tree.resetFall();
            }
        }
        else
        {
            ESP_LOGE(TAG, "Erro mpu.update(): %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
