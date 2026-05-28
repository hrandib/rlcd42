/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#include "screen.hpp"
#include "display_bsp.h"
#include "i2c_bus.hpp"
#include "u8g2_drawables.hpp"

#include "esp_sleep.h"
#include "esp_system.h"
#include "user_config.h"
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

static const char* TAG = "screen";

static esp_reset_reason_t get_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();

    switch(reason) {
    case ESP_RST_POWERON:
        printf("Reset by power-on\n");
        break;
    case ESP_RST_SW:
        printf("Software reset\n");
        break;
    case ESP_RST_PANIC:
        printf("Reset by panic/exception\n");
        break;
    case ESP_RST_TASK_WDT:
        printf("Reset by Task Watchdog\n");
        break;
    case ESP_RST_BROWNOUT:
        printf("Reset by Brownout\n");
        break;
    case ESP_RST_DEEPSLEEP:
        printf("Wake up from Deep Sleep\n");
        break;
    default:
        printf("Other reset reason: %d\n", reason);
        break;
    }
    return reason;
}

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

    if(get_reset_reason() == ESP_RST_DEEPSLEEP) {
        config.initialized = true; // Skip initialization if waking up from deep sleep to preserve display state
    }

    ESP_RETURN_ON_ERROR(u8g2_st7305_init(&g_u8g2_lcd_, &config), TAG, "Failed to initialize u8g2 display");
    u8g2_ = u8g2_st7305_get_u8g2(&g_u8g2_lcd_);
    u8g2_SetDrawColor(u8g2_, 1);

    return ESP_OK;
}

void draw_grid(U8g2Drawables& drawer)
{
    drawer.DrawHLineCentered(100, LCD_HEIGHT / 2, 3);
    drawer.DrawVLineCentered(LCD_WIDTH / 2, 70, 3);
    drawer.DrawHLineCentered(0, -20, 2);
}

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5          /* Time ESP32 will go to sleep (in seconds) */

void Screen::display_task(void* screen_instance)
{
    Screen* screen = static_cast<Screen*>(screen_instance);
    u8g2_t* u8g2 = screen->u8g2_;
    Shtc3Async& shtc3 = screen->shtc3_;
    while(true) {
        u8g2_ClearBuffer(u8g2);
        U8g2Drawables drawer(u8g2);
        draw_grid(drawer);

        // Read sensor data
        Shtc3Async::SensorData sdata;
        if(shtc3.read(sdata) == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.1f °C, Humidity: %.1f %%", sdata.temperature, sdata.humidity);
        }

        // Display sensor values
        drawer.SetFont(u8g2_font_logisoso78_tn);
        char text[80];
        snprintf(text, sizeof(text), "%.1f", sdata.temperature);
        drawer.DrawStr(5, 120, text);
        snprintf(text, sizeof(text), "%.1f", sdata.humidity);
        drawer.DrawStr(-180, 120, text);

        drawer.SetFont(u8g2_font_crox5hb_tf);
        drawer.DrawCenteredStrUtf8(100, 30, "°C");
        drawer.DrawCenteredStrUtf8(-100, 30, "%H");

        drawer.SetFont(u8g2_font_siji_t_6x10);
        drawer.DrawGlyph(-52, -6, 0xE210);
        drawer.SetFont(u8g2_font_profont17_mr);
        drawer.DrawStr(-40, -3, "100%");

        u8g2_SendBuffer(u8g2);
        vTaskDelay(pdMS_TO_TICKS(10000));
        // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
        // esp_deep_sleep_start();
    }
    vTaskDelete(NULL);
}

void Screen::init()
{
    init_display();
    BaseType_t ok = xTaskCreatePinnedToCore(display_task, "screen", 8192, this, 4, NULL, 1);
    configASSERT(ok == pdPASS);
}
