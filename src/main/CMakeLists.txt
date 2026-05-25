idf_component_register(
    SRCS "i2c_scan.c" "main.cpp"
    REQUIRES
        esp_timer
        port_bsp
        u8g2_st7305
        shtc3_cpp
        esp_driver_i2c
    INCLUDE_DIRS "./")
