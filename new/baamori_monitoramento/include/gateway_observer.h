#pragma once
#include "sendFall.h"
#include "fall_observer.h"

class GatewayObserver : public FallObserver {
public:
    void onFall( DataStatistics& stats, uint32_t fall_id, int is_fall, const char* device_id) {
        if (is_fall == 1) {
            send_fall_to_gateway(stats, fall_id, is_fall, device_id);
        }
    }
};
