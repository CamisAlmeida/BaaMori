#ifndef CLIENT_HTTP_H
#define CLIENT_HTTP_H

#include "esp_http_client.h"

void http_post(const char *url, const char *json)
{
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .keep_alive_enable = true
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, strlen(json));

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

#endif