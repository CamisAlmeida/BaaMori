#pragma once
#include "dataStatistics.h"
#include <time.h>
#include <string>

class Queda
{
private:
    time_t timestamp;          
    DataStatistics statistics; 

public:
    Queda() : timestamp(0) {}

    Queda(const DataStatistics& stats)
        : statistics(stats)
    {
        timestamp = time(nullptr);
    }

    void registrarQueda(const DataStatistics& stats)
    {
        statistics = stats;
        timestamp = time(nullptr);
    }

    time_t getTimestamp() const { return timestamp; }

    const DataStatistics& getStatistics() const { return statistics; }

    std::string getTimestampString() const
    {
        if (timestamp == 0) return "Sem registro";

        struct tm info;
        localtime_r(&timestamp, &info);

        char buffer[40];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &info);
        return std::string(buffer);
    }
};
