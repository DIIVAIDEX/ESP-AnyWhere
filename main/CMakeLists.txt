idf_component_register(SRCS	"main.c"
                    PRIV_REQUIRES nvs_flash iot_usbh_modem Storage OTA SoftAP_STA WebServer Telemetry esp_wireguard esp_pm)
