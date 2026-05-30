#include "driver/gpio.h"
#include "user_config.h"
#include <concepts>

consteval uint64_t gpio_pinlist_to_mask(std::convertible_to<gpio_num_t> auto... pins)
{
    uint64_t mask = 0;
    ((mask |= (1ULL << pins)), ...);
    return mask;
}

static inline void set_unused_pin_config()
{
    static const char* TAG = "GPIO_DEFAULTS";

    uint64_t pulldown_pins =
      gpio_pinlist_to_mask(I2S_DSDIN_PIN, I2S_SCLK_PIN, I2S_ASDOUT_PIN, I2S_LRCK_PIN, PA_CTL_PIN);

    gpio_config_t io_conf{};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = pulldown_pins;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    if(gpio_config(&io_conf) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure unused pins");
    }
}