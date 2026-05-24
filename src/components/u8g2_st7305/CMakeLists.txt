idf_component_register(
    SRCS "u8g2_st7305.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_driver_spi
        esp_driver_gpio
        u8g2
)
