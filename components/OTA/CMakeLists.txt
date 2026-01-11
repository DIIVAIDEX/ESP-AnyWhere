idf_component_register(SRCS "OTA.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_http_client app_update esp_https_ota mbedtls
                       EMBED_TXTFILES ${project_dir}/components/OTA/server_certs/isrgrootx1.pem)

