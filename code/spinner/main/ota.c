#include <string.h>

#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_wifi.h>

#include "all_things.h"

#define WIFI_SSID "ESP32 OTA Update"

/*
 * Serve OTA update portal (index.html)
 */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

extern const uint8_t ota_html_start[] asm("_binary_ota_html_start");
extern const uint8_t ota_html_end[] asm("_binary_ota_html_end");

esp_err_t pics_post_handler(httpd_req_t *req);
esp_err_t pics_post_handler_fall(httpd_req_t *req);
esp_err_t pics_post_handler_full(httpd_req_t *req, int input);
esp_err_t ota_get_handler(httpd_req_t *req);



esp_err_t index_get_handler(httpd_req_t *req)
{
    esp_err_t ret;
    FILE      *f;
    size_t      ret_sz;
    char     buf[64];
    char     pth[524];
    bool reading = TRUE;

    if( strcmp( req->uri, "/ota" ) == 0 )
        {
        return ota_get_handler( req );
        }

    if( strcmp( req->uri, "/fall" ) == 0 )
        {
        httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
        return ESP_OK;
        }

    sd_open();
    printf("Hello, * handler!  %s\n", req->uri);
    if( !req->uri[1] )
        {
        snprintf( pth, sizeof(pth), MOUNT_POINT"/index.htm");
        }
    else
        {
        snprintf( pth, sizeof(pth), MOUNT_POINT"%s", req->uri );
        }
    f = fopen(pth, "rb");
    if (f == NULL) {
        printf("Failed to open: %s!\n", pth);
        sd_close();
        return( ESP_FAIL );
    }

    do
        {
        ret_sz = fread( buf, 1, sizeof(buf), f );
        printf("index ret_sz: %u\n", ret_sz);
         ret = httpd_resp_send_chunk(req, buf, ret_sz);
        }
    while( ret_sz > 0 );

    fclose(f);
    sd_close();


    return ESP_OK;
}

esp_err_t ota_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, (const char *) ota_html_start, ota_html_end - ota_html_start);
    return ESP_OK;
}


esp_err_t pics_post_handler(httpd_req_t *req)
{
    return pics_post_handler_full( req, 1);
}

esp_err_t pics_post_handler_fall(httpd_req_t *req)
{
    return pics_post_handler_full( req, 0);
}

esp_err_t pics_post_handler_full(httpd_req_t *req, int input)
{
    char *buf = g_buf;
    size_t buf_remain = G_BUF_SZ;
    int remaining = req->content_len;
    FILE      *f;
    char     pth[164];

    while (remaining > 0) {
        int recv_len = httpd_req_recv(req, buf, MIN(remaining, buf_remain));

        // Timeout Error: Just retry
        if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;

        // Serious Error: Abort OTA
        } else if (recv_len <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
            return ESP_FAIL;
        }

        remaining -= recv_len;
        buf_remain -= recv_len;
        buf += recv_len;
    }
    httpd_resp_sendstr(req, "PICS!!!\n");


    if( input == 1 )
        {
        /* Save image to SD card */
        sd_open();
        snprintf( pth, sizeof(pth), MOUNT_POINT"/new_pic.bin" );
        f = fopen(pth, "wb");
        if (f) {
            buf_remain = fwrite( g_buf, 1, sizeof(g_buf), f);
            printf("Opened: %s!   And wrote this much: %d\n", pth, buf_remain );
            fclose(f);
            }
        sd_close();
        }

    return ESP_OK;
}



/*
 * Handle OTA file upload
 */
esp_err_t update_post_handler(httpd_req_t *req)
{
    char buf[1000];
    esp_ota_handle_t ota_handle;
    int remaining = req->content_len;

    const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
    ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

    while (remaining > 0) {
        int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

        // Timeout Error: Just retry
        if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;

        // Serious Error: Abort OTA
        } else if (recv_len <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
            return ESP_FAIL;
        }

        // Successful Upload: Flash firmware chunk
        if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
            return ESP_FAIL;
        }

        remaining -= recv_len;
    }

    // Validate and switch to new OTA image and reboot
    if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
            return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_restart();

    return ESP_OK;
}

/*
 * HTTP Server
 */
httpd_uri_t index_get = {
    .uri      = "/*",
    .method   = HTTP_GET,
    .handler  = index_get_handler,
    .user_ctx = NULL
};

httpd_uri_t ota_get = {
    .uri      = "/ota",
    .method   = HTTP_GET,
    .handler  = ota_get_handler,
    .user_ctx = NULL
};

httpd_uri_t update_post = {
    .uri      = "/update",
    .method   = HTTP_POST,
    .handler  = update_post_handler,
    .user_ctx = NULL
};

httpd_uri_t pics_post = {
    .uri      = "/pics",
    .method   = HTTP_POST,
    .handler  = pics_post_handler,
    .user_ctx = NULL
};

httpd_uri_t pics_post_fall = {
    .uri      = "/pics_fall",
    .method   = HTTP_POST,
    .handler  = pics_post_handler_fall,
    .user_ctx = NULL
};

static esp_err_t http_server_init(void)
{
    static httpd_handle_t http_server = NULL;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&http_server, &config) == ESP_OK) {
        httpd_register_uri_handler(http_server, &index_get);
        // httpd_register_uri_handler(http_server, &ota_get);
        httpd_register_uri_handler(http_server, &update_post);
        httpd_register_uri_handler(http_server, &pics_post);
        httpd_register_uri_handler(http_server, &pics_post_fall);
    }

    return http_server == NULL ? ESP_FAIL : ESP_OK;
}

/*
 * WiFi configuration
 */
static esp_err_t softap_init(void)
{
    esp_err_t res = ESP_OK;

    res |= esp_netif_init();
    res |= esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    res |= esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 6,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 3
        },
    };

    res |= esp_wifi_set_mode(WIFI_MODE_AP);
    res |= esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    res |= esp_wifi_start();

    return res;
}

void ota_server(void* args) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(softap_init());
    ESP_ERROR_CHECK(http_server_init());

    /* Mark current app as valid */
    const esp_partition_t *partition = esp_ota_get_running_partition();
    printf("Currently running partition: %s\r\n", partition->label);

    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    // while(1) vTaskDelay(10);
}
