#pragma  once 

#include "fall_observer.h"

#define MAX_OBSERVERS 4

class FallSubject {
private:
    FallObserver* observers[MAX_OBSERVERS];
    int count = 0;

public:
    void notify( DataStatistics& stats, uint32_t fall_id, int is_fall, const char* device_id ){
        for (int i = 0; i < count; i++) {
            observers[i]->onFall(stats, fall_id, is_fall, device_id);
        }
    }
    
    void attach(FallObserver* obs)
    {
        if (count < MAX_OBSERVERS) {
            observers[count++] = obs;
        }
    }

    
};


