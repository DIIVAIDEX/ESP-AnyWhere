idf_component_register(SRCS "Telemetry.c"
							"LogsToWeb.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES WebServer esp_ringbuf esp_driver_tsens
                       REQUIRES Bat_MAX17048 iot_usbh_modem)

