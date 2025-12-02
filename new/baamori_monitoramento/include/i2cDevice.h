#pragma once
#include <stdint.h>
#include "driver/i2c.h"

#ifndef SDA_GPIO
#define SDA_GPIO 9
#endif

#ifndef SCL_GPIO
#define SCL_GPIO 10
#endif

#ifndef I2C_FREQ
#define I2C_FREQ 400000
#endif

class I2CDevice {
public:
    I2CDevice(i2c_port_t port, uint8_t address)
        : port(port), address(address) {}


    // INIT I2C
    void init()
    {
        i2c_config_t conf = {};
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = SDA_GPIO;
        conf.scl_io_num = SCL_GPIO;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = I2C_FREQ;

        i2c_param_config(port, &conf);
        i2c_driver_install(port, conf.mode, 0, 0, 0);
    }


    // WRITE 1 REG
    esp_err_t writeReg(uint8_t reg, uint8_t value)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg, true);
        i2c_master_write_byte(cmd, value, true);

        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(port, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
    }


    // READ MULTIPLE
    esp_err_t readBytes(uint8_t reg, uint8_t *data, size_t len)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg, true);

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true);
        i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(port, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
    }

protected:
    i2c_port_t port;
    uint8_t address;
};
