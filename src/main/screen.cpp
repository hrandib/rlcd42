/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#include "screen.hpp"
#include "display_bsp.h"
#include "i2c_bus.hpp"
#include "u8g2_drawables.hpp"

#include "user_config.h"
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

static const char* TAG = "screen";

esp_err_t Screen::init_display()
{
    u8g2_st7305_config_t config = u8g2_st7305_default_config();
    config.mosi_io = RLCD_MOSI_PIN;
    config.sclk_io = RLCD_SCK_PIN;
    config.dc_io = RLCD_DC_PIN;
    config.cs_io = RLCD_CS_PIN;
    config.reset_io = RLCD_RST_PIN;
    config.rotation = U8G2_R1;
    config.tile_buf_height = U8G2_ST7305_TILE_BUF_FULL;

    ESP_RETURN_ON_ERROR(u8g2_st7305_init(&g_u8g2_lcd_, &config), TAG, "Failed to initialize u8g2 display");
    u8g2_ = u8g2_st7305_get_u8g2(&g_u8g2_lcd_);
    u8g2_SetDrawColor(u8g2_, 1);

    return ESP_OK;
}

void Screen::display_task(void* screen_instance)
{
    Screen* screen = static_cast<Screen*>(screen_instance);
    u8g2_t* u8g2 = screen->u8g2_;
    Shtc3Async& shtc3 = screen->shtc3_;

    while(true) {

        u8g2_ClearBuffer(u8g2);

        u8g2_SetFont(u8g2, u8g2_font_logisoso58_tf);
        U8g2Drawables drawer(u8g2);

        // Draw decorative line
        drawer.DrawHLine(10, -30, -10, 3);

        // Calculate vertical center position for sensor data
        int number_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2);
        int number_y = ((LCD_HEIGHT - number_height) / 2) + u8g2_GetAscent(u8g2);

        // Read sensor data
        Shtc3Async::SensorData sdata;
        if(shtc3.read(sdata) == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.1f °C, Humidity: %.1f %%", sdata.temperature, sdata.humidity);
        }

        // Display sensor values
        char text[80];
        snprintf(text, sizeof(text), "%.1f  %.1f", sdata.temperature, sdata.humidity);
        drawer.DrawCenteredStr(number_y, text);

        u8g2_SendBuffer(u8g2);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    vTaskDelete(NULL);
}

void Screen::init()
{
    init_display();
    BaseType_t ok = xTaskCreatePinnedToCore(display_task, "screen", 8192, this, 4, NULL, 1);
    configASSERT(ok == pdPASS);
}
