/* Captive Portal Example

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include <sys/param.h>
#include "ComWS.h"
#include <lwip/sockets.h>
#include "WebServer.h"

extern const char index_start[] asm("_binary_index_html_gz_start");
extern const char index_end[] asm("_binary_index_html_gz_end");
extern const char style_start[] asm("_binary_style_css_gz_start");
extern const char style_end[] asm("_binary_style_css_gz_end");
extern const char scriptWS_start[] asm("_binary_scriptWS_js_gz_start");
extern const char scriptWS_end[] asm("_binary_scriptWS_js_gz_end");

static const char *TAG = "WebServer";

// HTTP GET Handler
static esp_err_t index_get_handler(httpd_req_t *req)
{
    const uint32_t index_len = index_end - index_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, index_start, index_len);

    return ESP_OK;
}

static esp_err_t style_get_handler(httpd_req_t *req)
{
    const uint32_t style_len = style_end - style_start;

    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, style_start, style_len);

    return ESP_OK;
}

static esp_err_t scriptWS_get_handler(httpd_req_t *req)
{
    const uint32_t scriptWS_len = scriptWS_end - scriptWS_start;

    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, scriptWS_start, scriptWS_len);

    return ESP_OK;
}

static esp_err_t favicon_get_handler(httpd_req_t *req)
{
    const uint32_t index_len = index_end - index_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_start, index_len);

    return ESP_OK;
}

// HTTP Error (404) Handler - Redirects all requests to the root page
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // Set status
    httpd_resp_set_status(req, "302 Temporary Redirect");
    // Redirect to the "/" root directory
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Redirecting to root");
    return ESP_OK;
}

static esp_err_t handle_ws_req(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);

	ws_parse(req, &ws_pkt);

    free(buf);
    return ESP_OK;
}

void WebServer_Start(void)
{	
//	memcpy(&comWsData, _comWsData, sizeof(comWsData_t));
	
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
//    config.max_open_sockets = 7;
//    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if(httpd_start(&server, &config) == ESP_OK){
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        
	    httpd_uri_t ws = {.uri = "/ws",
	        			  .method = HTTP_GET,
	        			  .handler = handle_ws_req,
	        			  .user_ctx = NULL,
	        			  .is_websocket = true};
		httpd_register_uri_handler(server, &ws);
		
        httpd_uri_t uriHandler = {.uri = "/style.css",
		    					  .method = HTTP_GET,
		    					  .handler = style_get_handler};
        httpd_register_uri_handler(server, &uriHandler);
        
		uriHandler.uri = "/scriptWS.js";
		uriHandler.handler = scriptWS_get_handler;
        httpd_register_uri_handler(server, &uriHandler);
        
		uriHandler.uri = "/";
		uriHandler.handler = index_get_handler;
        httpd_register_uri_handler(server, &uriHandler);
        
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
}
