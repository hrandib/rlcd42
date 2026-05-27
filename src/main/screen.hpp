/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include "sensor_desc.hpp"
#include "shtc3_async.hpp"
#include "u8g2.h"
#include "u8g2_st7305.h"

/**
 * @brief Manages the display and renders sensor data
 */
class Screen
{
public:
    Screen(Shtc3Async* shtc3) : shtc3_(*shtc3), u8g2_(nullptr)
    { }
    /**
     * @brief Initialize and start the display task
     */
    void init();
private:
    Shtc3Async& shtc3_;
    u8g2_st7305_t g_u8g2_lcd_;
    u8g2_t* u8g2_;

    /**
     * @brief Initialize display
     * @return ESP_OK if successful, otherwise an error code
     */
    esp_err_t init_display();

    /**
     * @brief Main display update task
     * @param screen_instance Pointer to Screen instance
     */
    static void display_task(void* screen_instance);
};
