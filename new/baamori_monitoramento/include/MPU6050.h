#pragma once
#include <stdint.h>
#include "I2CDevice.h"
#include "mpu6050_registers.h"
#include "esp_log.h"

class MPU6050 : public I2CDevice {
public:
    MPU6050(i2c_port_t port = I2C_NUM_0, uint8_t addr = MPU6050_ADDRESS)
        : I2CDevice(port, addr),
          accX(0), accY(0), accZ(0),
          gyroX(0), gyroY(0), gyroZ(0),
          temperatureC(0) {}


    bool begin()
    {
        uint8_t who = 0;
        if (readBytes(MPU6050_REG_WHO_AM_I, &who, 1) != ESP_OK)
            return false;

        ESP_LOGW("MPU", "WHO_AM_I = 0x%02X", who);

        if (who != 0x68 && who != 0x69)
            return false;

        writeReg(MPU6050_REG_PWR_MGMT_1, 0x00);
        writeReg(MPU6050_REG_ACCEL_CONFIG, 0x00);
        writeReg(MPU6050_REG_GYRO_CONFIG, 0x00);

        return true;
    }


    esp_err_t update()
    {
        uint8_t data[14];
        esp_err_t ret = readBytes(MPU6050_REG_ACCEL_XOUT_H, data, 14);
        if (ret != ESP_OK) return ret;

        accX = (int16_t)(data[0] << 8 | data[1]);
        accY = (int16_t)(data[2] << 8 | data[3]);
        accZ = (int16_t)(data[4] << 8 | data[5]);

        int16_t rawT = (int16_t)(data[6] << 8 | data[7]);
        temperatureC = rawT / 340.0f + 36.53f;

        gyroX = (int16_t)(data[8] << 8 | data[9]);
        gyroY = (int16_t)(data[10] << 8 | data[11]);
        gyroZ = (int16_t)(data[12] << 8 | data[13]);

        return ESP_OK;
    }


    int16_t getAccX() const { return accX; }
    int16_t getAccY() const { return accY; }
    int16_t getAccZ() const { return accZ; }

    int16_t getGyroX() const { return gyroX; }
    int16_t getGyroY() const { return gyroY; }
    int16_t getGyroZ() const { return gyroZ; }

    float getTemperatureC() const { return temperatureC; }


private:
    int16_t accX, accY, accZ;
    int16_t gyroX, gyroY, gyroZ;
    float temperatureC;
};
