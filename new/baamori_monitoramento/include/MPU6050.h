#pragma once
#include <stdint.h>

extern "C" {
    #include "driver/i2c.h"
}

#include "mpu6050_registers.h"

class MPU6050 {
public:
    MPU6050(i2c_port_t port = I2C_NUM_0, uint8_t address = MPU6050_ADDRESS);

    bool begin();
    esp_err_t update();

    int16_t getAccX() const { return accX; }
    int16_t getAccY() const { return accY; }
    int16_t getAccZ() const { return accZ; }

    int16_t getGyroX() const { return gyroX; }
    int16_t getGyroY() const { return gyroY; }
    int16_t getGyroZ() const { return gyroZ; }

    float getTemperatureC() const { return temperatureC; }

private:
    i2c_port_t port;
    uint8_t addr;

    int16_t accX, accY, accZ;
    int16_t gyroX, gyroY, gyroZ;
    float temperatureC;

    esp_err_t writeReg(uint8_t reg, uint8_t value);
    esp_err_t readBytes(uint8_t reg, uint8_t *data, size_t len);
};
