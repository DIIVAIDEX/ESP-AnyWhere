idf_component_register(SRCS 			"WebServer.c"
										"ComWS.c"
                       PRIV_REQUIRES	esp_event cjson Telemetry OTA freeRTOS_Stats SoftAP_STA Storage esp_psram
                       INCLUDE_DIRS 	"include"
                       REQUIRES			esp_http_server
                       EMBED_FILES		wwwroot/index.html.gz
										wwwroot/style.css.gz
	                       			   	wwwroot/scriptWS.js.gz)

