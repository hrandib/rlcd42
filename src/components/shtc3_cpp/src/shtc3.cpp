/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#include "shtc3.hpp"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "SHTC3";

// SHTC3 Register Addresses
#define SHTC3_REG_WAKE          0x3517
#define SHTC3_REG_SLEEP         0xB098
#define SHTC3_REG_SOFT_RESET    0x805D
#define SHTC3_REG_READ_ID       0xEFC8

// Measurement registers
#define SHTC3_REG_T_CSE_NM      0x7CA2  // Temperature first, clock stretching enabled, normal mode
#define SHTC3_REG_T_CSD_NM      0x7866  // Temperature first, clock stretching disabled, normal mode
#define SHTC3_REG_T_CSE_LM      0x6458  // Temperature first, clock stretching enabled, low power mode
#define SHTC3_REG_T_CSD_LM      0x609C  // Temperature first, clock stretching disabled, low power mode

#define SHTC3_WARMUP_US                     240 // Warm-up time in microseconds
#define SHTC3_MEASURE_DELAY_NORMAL_MS       13  // Measurement delay in milliseconds
#define SHTC3_MEASURE_DELAY_LOW_POWER_MS    1   // Measurement delay in milliseconds

Shtc3::Shtc3(i2c_master_bus_handle_t bus_handle, uint32_t dev_speed, MeasurementMode mode)
    : bus_handle_(bus_handle), dev_handle_(nullptr), dev_speed_(dev_speed), measurement_mode_(mode), initialized_(false)
{
}

Shtc3::~Shtc3()
{
    if (initialized_) {
        deinit();
    }
}

esp_err_t Shtc3::init()
{
    esp_err_t ret;

    if (initialized_) {
        ESP_LOGW(TAG, "SHTC3 already initialized");
        return ESP_OK;
    }

    // Configure I2C device
    i2c_device_config_t dev_cfg{};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = DEV_ADDR;
    dev_cfg.scl_speed_hz = dev_speed_;

    // Add device to I2C bus
    ret = i2c_master_bus_add_device(bus_handle_, &dev_cfg, &dev_handle_);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to add SHTC3 device to I2C bus");

    initialized_ = true;
    ESP_LOGI(TAG, "SHTC3 initialized successfully");

    return ESP_OK;
}

esp_err_t Shtc3::deinit()
{
    esp_err_t ret;

    if (!initialized_) {
        ESP_LOGW(TAG, "SHTC3 not initialized");
        return ESP_OK;
    }

    ret = i2c_master_bus_rm_device(dev_handle_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove SHTC3 device from I2C bus");
        return ret;
    }

    dev_handle_ = nullptr;
    initialized_ = false;
    ESP_LOGI(TAG, "SHTC3 deinitialized successfully");

    return ESP_OK;
}

esp_err_t Shtc3::read(SensorData& data)
{
    esp_err_t ret;
    uint8_t b_read[6] = {0};
    uint16_t reg_addr = get_measurement_register(measurement_mode_);
    uint8_t read_reg[2] = {(uint8_t)(reg_addr >> 8), (uint8_t)(reg_addr & 0xff)};

    ESP_RETURN_ON_FALSE(initialized_ && dev_handle_, ESP_ERR_INVALID_STATE, TAG,
                        "SHTC3 not initialized");

    // Wake up sensor
    ret = wake();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to wake up SHTC3 sensor");

    // Send measurement command
    ret = i2c_master_transmit(dev_handle_, read_reg, 2, 200);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to send measurement command");

    // Wait for measurement to complete
    vTaskDelay(pdMS_TO_TICKS(measurement_mode_ == MeasurementMode::NORMAL_MODE ? SHTC3_MEASURE_DELAY_NORMAL_MS : SHTC3_MEASURE_DELAY_LOW_POWER_MS));

    // Read measurement data
    ret = i2c_master_receive(dev_handle_, b_read, 6, 200);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to read data from SHTC3 sensor");

    // Put sensor to sleep
    ret = sleep();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to put SHTC3 sensor to sleep");

    // Convert raw data to temperature and humidity
    data.temperature = ((((b_read[0] * 256.0) + b_read[1]) * 175) / 65535.0) - 45;
    data.humidity = ((((b_read[3] * 256.0) + b_read[4]) * 100) / 65535.0);

    return ESP_OK;
}

esp_err_t Shtc3::read_id(uint8_t *id)
{
    esp_err_t ret;
    uint8_t b_read[2] = {0};
    uint8_t read_reg[2] = {SHTC3_REG_READ_ID >> 8, SHTC3_REG_READ_ID & 0xff};

    ESP_RETURN_ON_FALSE(id, ESP_ERR_INVALID_ARG, TAG, "Invalid pointer: id cannot be NULL");
    ESP_RETURN_ON_FALSE(initialized_ && dev_handle_, ESP_ERR_INVALID_STATE, TAG,
                        "SHTC3 not initialized");

    ret = wake();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to wake up SHTC3 sensor");

    ret = i2c_master_transmit_receive(dev_handle_, read_reg, 2, b_read, 2, 200);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to read SHTC3 ID");

    id[0] = b_read[0];
    id[1] = b_read[1];

    ret = sleep();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to put SHTC3 sensor to sleep");

    return ESP_OK;
}

esp_err_t Shtc3::wake()
{
    esp_err_t ret;
    uint8_t wake_reg[2] = {SHTC3_REG_WAKE >> 8, SHTC3_REG_WAKE & 0xff};

    ESP_RETURN_ON_FALSE(initialized_ && dev_handle_, ESP_ERR_INVALID_STATE, TAG,
                        "SHTC3 not initialized");

    ret = i2c_master_transmit(dev_handle_, wake_reg, 2, -1);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to wake up SHTC3 sensor");

    esp_rom_delay_us(SHTC3_WARMUP_US);

    return ESP_OK;
}

esp_err_t Shtc3::sleep()
{
    esp_err_t ret;
    uint8_t sleep_reg[2] = {SHTC3_REG_SLEEP >> 8, SHTC3_REG_SLEEP & 0xff};

    ESP_RETURN_ON_FALSE(initialized_ && dev_handle_, ESP_ERR_INVALID_STATE, TAG,
                        "SHTC3 not initialized");

    ret = i2c_master_transmit(dev_handle_, sleep_reg, 2, -1);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to put SHTC3 sensor to sleep");

    return ESP_OK;
}

esp_err_t Shtc3::reset()
{
    esp_err_t ret;
    uint8_t reset_reg[2] = {SHTC3_REG_SOFT_RESET >> 8, SHTC3_REG_SOFT_RESET & 0xff};

    ESP_RETURN_ON_FALSE(initialized_ && dev_handle_, ESP_ERR_INVALID_STATE, TAG,
                        "SHTC3 not initialized");

    ret = i2c_master_transmit(dev_handle_, reset_reg, 2, -1);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to reset SHTC3 sensor");

    esp_rom_delay_us(SHTC3_WARMUP_US);

    return ESP_OK;
}

uint16_t Shtc3::get_measurement_register(MeasurementMode mode) const
{
    if (mode == MeasurementMode::NORMAL_MODE) {
        if (CLOCK_STRETCHING_MODE == ClockStretchingMode::ENABLED) {
            return SHTC3_REG_T_CSE_NM;
        } else {
            return SHTC3_REG_T_CSD_NM;
        }
    } else {  // LOW_POWER_MODE
        if (CLOCK_STRETCHING_MODE == ClockStretchingMode::ENABLED) {
            return SHTC3_REG_T_CSE_LM;
        } else {
            return SHTC3_REG_T_CSD_LM;
        }
    }
}
