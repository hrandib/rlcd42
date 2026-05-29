/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include "esp_log.h"
#include "isensordatasource.hpp"
#include "shtc3.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

/**
 * @brief Asynchronous wrapper for SHTC3 using std::thread
 */
class Shtc3Async : public Shtc3, public ISensorDataSource
{
public:
    using Shtc3::read;
    using DataCallback = std::function<void(const SensorData&)>;

    Shtc3Async(i2c_master_bus_handle_t bus_handle,
               uint32_t dev_speed,
               MeasurementMode mode = MeasurementMode::NORMAL_MODE) :
      Shtc3(bus_handle, dev_speed, mode), ISensorDataSource("SHTC3"), running_(false)
    {
        store_item("Temperature", "°C", "%.1f");
        store_item("Humidity", "%", "%.1f");
    }

    ~Shtc3Async()
    {
        stop();
    }

    /**
     * @brief Start reading asynchronously in the background.
     *
     * @param on_data Lambda/Callback invoked when data is successfully read.
     * @param interval_ms Polling interval in milliseconds (default: 1000ms).
     */
    void read(DataCallback on_data, uint32_t interval_ms = 1000)
    {
        stop(); // Ensure any existing thread is stopped before starting a new one
        running_ = true;

        worker_ = std::thread([this, on_data, interval_ms]() {
            while(running_) {
                Shtc3::SensorData data;
                if(Shtc3::read(data) == ESP_OK) {
                    if(on_data) {
                        on_data(data);
                    }
                }
                if(running_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                }
            }
        });
    }

    /**
     * @brief Blocks the calling thread until the background reader finishes.
     */
    void join()
    {
        if(worker_.joinable()) {
            worker_.join();
        }
    }

    /**
     * @brief Stops the background reading task.
     */
    void stop()
    {
        running_ = false;
        if(worker_.joinable()) {
            worker_.join();
        }
    }

    const SensorDataArray& read_values() override
    {
        SensorData values;
        esp_err_t err = Shtc3::read(values);
        if(err != ESP_OK) {
            ESP_LOGE("Shtc3Async", "Failed to read sensor data: %s", esp_err_to_name(err));
            return data_;
        }
        update_item("Temperature", values.temperature);
        update_item("Humidity", values.humidity);
        return data_;
    }
private:
    std::thread worker_;
    std::atomic<bool> running_;
};