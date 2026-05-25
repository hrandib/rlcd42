/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#include "i2c_bus.hpp"
#include "esp_log.h"
#include <cstdio>

static const char* TAG = "i2c_bus";

I2CBus::I2CBus(gpio_num_t scl_io, gpio_num_t sda_io, int i2c_port, bool enable_pullup)
{
    ESP_LOGI(TAG, "Initializing I2C Master bus (SCL: %d, SDA: %d)...", scl_io, sda_io);

    i2c_master_bus_config_t i2c_mst_config{};

    i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_mst_config.i2c_port = i2c_port;
    i2c_mst_config.scl_io_num = scl_io;
    i2c_mst_config.sda_io_num = sda_io;
    i2c_mst_config.glitch_ignore_cnt = 7;
    i2c_mst_config.flags.enable_internal_pullup = enable_pullup;

    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &handle_);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master bus: %s", esp_err_to_name(ret));
        handle_ = nullptr;
    }
    else {
        ESP_LOGI(TAG, "I2C Master bus initialized successfully");
    }
}

I2CBus::~I2CBus()
{
    if(handle_ != nullptr) {
        esp_err_t ret = i2c_del_master_bus(handle_);
        if(ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(ret));
        }
        else {
            ESP_LOGI(TAG, "I2C Master bus deleted");
        }
        handle_ = nullptr;
    }
}

esp_err_t I2CBus::probe(uint8_t addr, uint32_t timeout_ms)
{
    if(!is_initialized()) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return ESP_FAIL;
    }
    return i2c_master_probe(handle_, addr, timeout_ms);
}

void I2CBus::scan()
{
    if(!is_initialized()) {
        ESP_LOGE(TAG, "I2C bus not initialized, cannot scan");
        return;
    }

    ESP_LOGI(TAG, "Starting I2C scan...");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for(int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for(int j = 0; j < 16; j++) {
            int addr = i + j;

            // i2cdetect typically skips addresses 0x00-0x02 and 0x78-0x7F
            if(addr < 0x03 || addr > 0x77) {
                printf("   ");
                continue;
            }

            // Probe the address with a 50ms timeout
            esp_err_t ret = probe(addr, 50);

            if(ret == ESP_OK) {
                printf("%02x ", addr); // Device found!
            }
            else if(ret == ESP_ERR_TIMEOUT) {
                printf("UU "); // Timeout (bus stuck or pulled low)
            }
            else {
                printf("-- "); // No device responding (NACK)
            }
        }
        printf("\n");
    }

    ESP_LOGI(TAG, "I2C scan completed");
}
