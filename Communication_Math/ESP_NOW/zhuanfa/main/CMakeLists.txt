idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES esp_driver_usb_serial_jtag esp_wifi esp_netif esp_event driver esp_hw_support esp_system esp_common nvs_flash usb esp_driver_uart
                    INCLUDE_DIRS ".") 