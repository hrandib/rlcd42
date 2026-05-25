#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "i2c_scan.h"

static const char *TAG = "i2c_scanner";

// I2C GPIO Configuration (Update these to match your board)
#define I2C_MASTER_SDA_IO   13      // GPIO for I2C Data
#define I2C_MASTER_SCL_IO   14      // GPIO for I2C Clock
#define I2C_PORT_NUM        -1      // -1 auto-allocates an available I2C port

/**
 * @brief I2C Scanner function formatted like Linux i2cdetect
 *
 * @param bus_handle Initialized I2C master bus handle
 */
void i2c_scanner(i2c_master_bus_handle_t bus_handle) {
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            int addr = i + j;

            // i2cdetect typically skips addresses 0x00-0x02 and 0x78-0x7F
            if (addr < 0x03 || addr > 0x77) {
                printf("   ");
                continue;
            }

            // Probe the address with a 50ms timeout
            esp_err_t ret = i2c_master_probe(bus_handle, addr, 50);

            if (ret == ESP_OK) {
                printf("%02x ", addr);          // Device found!
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");                  // Timeout (bus stuck or pulled low)
            } else {
                printf("-- ");                  // No device responding (NACK)
            }
        }
        printf("\n");
    }
}

i2c_master_bus_handle_t scan_main(void) {
    ESP_LOGI(TAG, "Initializing I2C Master bus...");

    // 1. Configure the I2C bus parameters
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true, // Fallback if external pull-ups are missing
    };

    i2c_master_bus_handle_t bus_handle;

    // 2. Initialize the master bus
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    ESP_LOGI(TAG, "Starting I2C scan...");

    // 3. Run the scanner
    i2c_scanner(bus_handle);

    // Optional: Delete the bus if you don't plan to use it further in your code
    // ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    return bus_handle;
}