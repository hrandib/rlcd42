
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "esp_check.h"

#include "display_bsp.h"
#include "user_config.h"
#include "u8g2_st7305.h"
#include "driver/i2c_types.h"
#include "i2c_scan.h"
#include "byteswap.h"
#include "shtc3_async.hpp"

static u8g2_st7305_t g_u8g2_lcd;
static const char *TAG = "rlcd42";

static void U8g2_DrawCenteredStr(u8g2_t *u8g2, int y, const char *text)
{
    int text_width = (int)u8g2_GetStrWidth(u8g2, text);
    int x = (LCD_WIDTH - text_width) / 2;
    if (x < 0)
    {
        x = 0;
    }
    u8g2_DrawStr(u8g2, x, y, text);
}

static void U8g2_DisplayTask(u8g2_t *u8g2, Shtc3Async &shtc3)
{
    uint32_t counter = 0;
    uint32_t frames = 0;
    uint32_t last_report_frames = 0;
    int64_t last_report_us = esp_timer_get_time();
    uint32_t fps_x100 = 0;
    uint32_t frame_us = 0;
    uint32_t flush_us = 0;

    std::mutex mtx;
    Shtc3Async::SensorData sdata;
    shtc3.read([&sdata, &mtx](auto data) {
        ESP_LOGI(TAG, "Temperature: %.1f °C, Humidity: %.1f %%", data.temperature, data.humidity);
        std::lock_guard<std::mutex> lock(mtx);
        sdata = data;
    });

    while (true)
    {
        const int64_t frame_start_us = esp_timer_get_time();

            u8g2_ClearBuffer(u8g2);
            u8g2_SetDrawColor(u8g2, 1);

            u8g2_SetFont(u8g2, u8g2_font_logisoso58_tf);

            u8g2_DrawFrame(u8g2, 10, 10, 380, 280);
            u8g2_DrawFrame(u8g2, 9, 9, 382, 282);
            u8g2_DrawHLine(u8g2, 18, 260, 364);

            int number_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2);
            int number_y = ((LCD_HEIGHT - number_height) / 2) + u8g2_GetAscent(u8g2);
            char text[80];
            {
                std::lock_guard<std::mutex> lock(mtx);
                snprintf(text, sizeof(text),  "%.1f  %.1f", sdata.temperature, sdata.humidity);
            }
            U8g2_DrawCenteredStr(u8g2, number_y, text);

            const int64_t flush_start_us = esp_timer_get_time();
            u8g2_SendBuffer(u8g2);
            const int64_t now_us = esp_timer_get_time();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    shtc3.stop();
    shtc3.join();
}

static void U8g2_CounterTask(void *)
{
    u8g2_st7305_config_t config = u8g2_st7305_default_config();
    config.mosi_io = RLCD_MOSI_PIN;
    config.sclk_io = RLCD_SCK_PIN;
    config.dc_io = RLCD_DC_PIN;
    config.cs_io = RLCD_CS_PIN;
    config.reset_io = RLCD_RST_PIN;
    config.rotation = U8G2_R1;
    config.tile_buf_height = U8G2_ST7305_TILE_BUF_FULL;

    ESP_ERROR_CHECK(u8g2_st7305_init(&g_u8g2_lcd, &config));
    u8g2_t *u8g2 = u8g2_st7305_get_u8g2(&g_u8g2_lcd);

    auto bus_handle = scan_main();

    Shtc3Async shtc3(bus_handle, ESP32_I2C_CLK_SPEED_HZ, Shtc3::MeasurementMode::NORMAL_MODE);
    esp_err_t init_ret = shtc3.init();
    ESP_RETURN_VOID_ON_ERROR(init_ret, TAG, "Failed to initialize SHTC3 device");
    ESP_LOGI(TAG, "Sensor initialization success");

    U8g2_DisplayTask(u8g2, shtc3);

    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    BaseType_t ok = xTaskCreatePinnedToCore(U8g2_CounterTask, "rlcd42_main", 8192, NULL, 4, NULL, 1);
    configASSERT(ok == pdPASS);
}
