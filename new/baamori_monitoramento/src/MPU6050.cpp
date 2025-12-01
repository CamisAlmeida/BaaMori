#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MPU6050.h"

#define SDA_GPIO 9
#define SCL_GPIO 10
#define I2C_FREQ 400000

static const char *TAG = "MAIN";

// I2C INIT 
void i2c_init()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_GPIO;
    conf.scl_io_num = SCL_GPIO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ;

    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

// APP MAIN 
extern "C" void app_main()
{
    ESP_LOGI(TAG, "Inicializando I2C...");
    i2c_init();

    MPU6050 mpu(I2C_NUM_0, 0x69);

    ESP_LOGI(TAG, "Iniciando MPU...");
    if (!mpu.begin()) {
        ESP_LOGE(TAG, "MPU NÃO encontrado!");
        return;
    }

    ESP_LOGI(TAG, "MPU inicializado com sucesso!");

    while (true)
    {
        if (mpu.update() == ESP_OK)
        {
            ESP_LOGI(TAG,
                     "ACC: %d %d %d | GYRO: %d %d %d | TEMP: %.2f",
                     mpu.getAccX(), mpu.getAccY(), mpu.getAccZ(),
                     mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ(),
                     mpu.getTemperatureC());
        }
        else
        {
            ESP_LOGE(TAG, "Erro ao ler dados do MPU!");
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
