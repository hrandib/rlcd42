
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_check.h"
#include "i2c_bus.hpp"
#include "screen.hpp"
#include "shtc3_async.hpp"

static const char* TAG = "main";

static std::unique_ptr<Shtc3Async> initialize_sensor(i2c_master_bus_handle_t bus_handle)
{
    auto shtc3 = std::make_unique<Shtc3Async>(bus_handle, ESP32_I2C_CLK_SPEED_HZ, Shtc3::MeasurementMode::NORMAL_MODE);
    if(shtc3->init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SHTC3 sensor");
        return nullptr;
    }
    return shtc3;
}

extern "C" void app_main(void)
{

    I2CBus i2c_bus(ESP32_I2C_SCL_PIN, ESP32_I2C_SDA_PIN);
    i2c_bus.scan();

    auto shtc3 = initialize_sensor(i2c_bus);
    if(!shtc3) {
        ESP_LOGE(TAG, "Sensor initialization failed, halting");
        return;
    }

    Screen screen(shtc3.get());
    screen.init();
    while(true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
