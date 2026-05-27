idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        esp_timer
        port_bsp
        u8g2_st7305
        shtc3_cpp
        i2c_helpers
        u8g2_helpers
    INCLUDE_DIRS "./")
