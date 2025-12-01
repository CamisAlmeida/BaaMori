#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "MPU6050.h"

#define I2C_PORT I2C_NUM_0
#define SDA_PIN 9     // Você disse que funcionam no Arduino IDE
#define SCL_PIN 10
static const char *TAG = "MAIN";

extern "C" void app_main() {
    ESP_LOGI(TAG, "Inicializando I2C...");

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.scl_io_num = SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);

    ESP_LOGI(TAG, "I2C OK!");

    MPU6050 mpu(I2C_PORT, 0x69);

    if (!mpu.begin()) {
        ESP_LOGE(TAG, "MPU6050 NAO DETECTADO! (endereco 0x69)");
        while (1) { vTaskDelay(1000 / portTICK_PERIOD_MS); }
    }

    ESP_LOGI(TAG, "MPU6050 iniciado!");

    // Loop 
    while (1) {
        if (mpu.update() == ESP_OK) {
            ESP_LOGI(TAG, "ACC: %d %d %d | GYRO: %d %d %d | TEMP: %.2f",
         mpu.getAccX(), mpu.getAccY(), mpu.getAccZ(),
         mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ(),
         mpu.getTemperatureC());

        } else {
            ESP_LOGE(TAG, "Erro lendo dados!");
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
