idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        esp_timer
        port_bsp
        u8g2_st7305
    INCLUDE_DIRS "./")
