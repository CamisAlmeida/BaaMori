#pragma once

#include "dataStatistics.h"

class FallObserver {
public:
    virtual void onFall(
        DataStatistics& stats,
        uint32_t fall_id,
        int is_fall,
        const char* device_id
    ) = 0;

    virtual ~FallObserver() {}
};
