#pragma once
#include "fall_observer.h"
#include "thingsboard.h"

class ThingsBoardObserver : public FallObserver {
public:
    void onFall( DataStatistics& stats, uint32_t fall_id, int is_fall, const char* device_id) 
    { send_fall(stats, fall_id, is_fall, device_id);}
};
