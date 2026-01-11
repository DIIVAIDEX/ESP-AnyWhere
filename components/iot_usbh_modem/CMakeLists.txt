set(srcs "src/esp_modem.c"
        "src/esp_modem_dce_command_lib.c"
        "src/esp_modem_dce_common_commands.c"
        "src/esp_modem_dce.c"
        "src/esp_modem_usb_dte.c"
        "src/esp_modem_netif.c"
        "src/esp_modem_recov_helper.c"
        "src/usbh_modem_board.c")

idf_component_register(	SRCS "${srcs}"
							 "iot_usbh_modem.c"
                    	INCLUDE_DIRS "include"
                    	PRIV_INCLUDE_DIRS "private_include"
                    	REQUIRES esp_netif
                    	PRIV_REQUIRES iot_usbh_cdc esp_event esp_driver_uart esp_driver_gpio)
