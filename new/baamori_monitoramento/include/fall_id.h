#include "nvs.h"
#include "nvs_flash.h"

#define NVS_NAMESPACE "storage"
#define NVS_KEY_FALL_ID "fall_id"

uint32_t load_fall_id()
{
    nvs_handle_t handle;
    uint32_t fall_id = 0;

    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) == ESP_OK) {
        nvs_get_u32(handle, NVS_KEY_FALL_ID, &fall_id);
        nvs_close(handle);
    }

    return fall_id;
}

void save_fall_id(uint32_t fall_id)
{
    nvs_handle_t handle;

    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_u32(handle, NVS_KEY_FALL_ID, fall_id);
        nvs_commit(handle);
        nvs_close(handle);
    }
}


