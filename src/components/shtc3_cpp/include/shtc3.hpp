/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <cstdint>

/**
 * @brief SHTC3 Temperature and Humidity Sensor C++ Wrapper
 *
 * This class provides a C++ interface to the SHTC3 I2C humidity and temperature sensor.
 */
class Shtc3
{
public:
    /**
     * @brief Measurement modes for the SHTC3 sensor
     */
    enum class MeasurementMode {
        NORMAL_MODE,   // Normal mode (higher accuracy, more current)
        LOW_POWER_MODE // Low power mode (lower accuracy, less current)
    };

    struct SensorData
    {
        float temperature; // Temperature in Celsius
        float humidity;    // Relative humidity in percentage
    };

    /**
     * @brief Constructor
     *
     * @param bus_handle I2C master bus handle
     * @param dev_speed I2C communication speed in Hz
     * @param mode Measurement mode (normal or low power)
     */
    Shtc3(i2c_master_bus_handle_t bus_handle, uint32_t dev_speed, MeasurementMode mode = MeasurementMode::NORMAL_MODE);

    /**
     * @brief Destructor
     *
     * Cleans up resources and removes device from I2C bus
     */
    ~Shtc3();

    /**
     * @brief Initialize the sensor
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t init();

    /**
     * @brief Deinitialize the sensor
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinit();

    /**
     * @brief Read temperature and humidity from the sensor
     *
     * @param temperature Pointer to store temperature in Celsius
     * @param humidity Pointer to store relative humidity in percentage
     * @param mode Measurement mode (normal or low power)
     * @param cs_mode Clock stretching mode
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t read(SensorData& data);

    /**
     * @brief Read sensor ID
     *
     * @param id Pointer to buffer for ID (2 bytes)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t read_id(uint8_t* id);

    /**
     * @brief Wake up the sensor
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t wake();

    /**
     * @brief Put sensor to sleep
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t sleep();

    /**
     * @brief Soft reset the sensor
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t reset();
private:
    /**
     * @brief Clock stretching configuration
     */
    enum class ClockStretchingMode {
        ENABLED, // Clock stretching enabled
        DISABLED // Clock stretching disabled
    };

    static constexpr uint8_t DEV_ADDR = 0x70; // Default I2C address for SHTC3
    static constexpr ClockStretchingMode CLOCK_STRETCHING_MODE =
      ClockStretchingMode::DISABLED; // Default clock stretching mode

    i2c_master_bus_handle_t bus_handle_;
    i2c_master_dev_handle_t dev_handle_;
    uint32_t dev_speed_;
    MeasurementMode measurement_mode_;
    bool initialized_;

    /**
     * @brief Get the register address for measurement
     *
     * @param mode Measurement mode
     * @return Register address (16-bit)
     */
    uint16_t get_measurement_register(MeasurementMode mode) const;
};
