/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "i2c_config.h"

/**
 * @brief C++ wrapper for ESP-IDF I2C Master bus
 */
class I2CBus
{
private:
    static constexpr int I2C_PORT_NUM = -1; // -1 auto-allocates an available I2C port
public:
    /**
     * @brief Constructor - initializes I2C master bus
     * @param scl_io GPIO number for SCL
     * @param sda_io GPIO number for SDA
     * @param i2c_port I2C port number (-1 for auto-allocation)
     * @param enable_pullup Whether to enable internal pull-ups
     */
    I2CBus(gpio_num_t scl_io, gpio_num_t sda_io, int i2c_port = I2C_PORT_NUM, bool enable_pullup = true);

    /**
     * @brief Destructor - cleans up the I2C bus
     */
    ~I2CBus();

    /**
     * @brief Check if the bus was initialized successfully
     * @return true if initialized, false otherwise
     */
    bool is_initialized() const
    {
        return handle_ != nullptr;
    }

    /**
     * @brief Get the I2C bus handle for manual operations
     * @return I2C master bus handle
     */
    i2c_master_bus_handle_t get_handle() const
    {
        return handle_;
    }

    /**
     * @brief Implicit conversion operator to I2C master bus handle
     */
    operator i2c_master_bus_handle_t() const
    {
        return handle_;
    }

    /**
     * @brief Scan I2C bus and print addresses of connected devices (linux i2cdetect style)
     */
    void scan();

    /**
     * @brief Probe a specific I2C address
     * @param addr I2C address to probe
     * @param timeout_ms Timeout in milliseconds
     * @return ESP_OK if device found, ESP_ERR_TIMEOUT if bus stuck, ESP_FAIL otherwise
     */
    esp_err_t probe(uint8_t addr, uint32_t timeout_ms = 50);
private:
    I2CBus(const I2CBus&) = delete;
    I2CBus& operator=(const I2CBus&) = delete;

    i2c_master_bus_handle_t handle_ = nullptr;
};
