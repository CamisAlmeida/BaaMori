#pragma once
#include <stdint.h>
#include <math.h>

class DataStatistics {
public:
    
    static DataStatistics& getInstance()
    {
        static DataStatistics instance; 
        return instance;
    }

    DataStatistics(const DataStatistics&) = delete;
    DataStatistics& operator=(const DataStatistics&) = delete;

    // Reset total
    void reset() {
        count = 0;

        resetAxis(sumAX, sumAY, sumAZ, sumGX, sumGY, sumGZ);
        resetAxis(sumSqAX, sumSqAY, sumSqAZ, sumSqGX, sumSqGY, sumSqGZ);

        initMinMax();

        magAccSum = 0.0f;
        magGyroSum = 0.0f;
    }

    // Atualiza estatísticas
    void update(float ax, float ay, float az, float gx, float gy, float gz) {
        count++;

        sumAX += ax;  sumAY += ay;  sumAZ += az;
        sumGX += gx;  sumGY += gy;  sumGZ += gz;

        sumSqAX += ax*ax;  sumSqAY += ay*ay;  sumSqAZ += az*az;
        sumSqGX += gx*gx;  sumSqGY += gy*gy;  sumSqGZ += gz*gz;

        updateMinMax(ax, minAX); updateMinMax(ay, minAY); updateMinMax(az, minAZ);
        updateMinMax(gx, minGX); updateMinMax(gy, minGY); updateMinMax(gz, minGZ);

        updateMax(ax, maxAX); updateMax(ay, maxAY); updateMax(az, maxAZ);
        updateMax(gx, maxGX); updateMax(gy, maxGY); updateMax(gz, maxGZ);

        magAccSum  += sqrtf(ax*ax + ay*ay + az*az);
        magGyroSum += sqrtf(gx*gx + gy*gy + gz*gz);
    }

    // Getters
    uint32_t getCount() const { return count; }

    float getMinAX() const { return minAX; }
    float getMinAY() const { return minAY; }
    float getMinAZ() const { return minAZ; }
    float getMinGX() const { return minGX; }
    float getMinGY() const { return minGY; }
    float getMinGZ() const { return minGZ; }

    float getMaxAX() const { return maxAX; }
    float getMaxAY() const { return maxAY; }
    float getMaxAZ() const { return maxAZ; }
    float getMaxGX() const { return maxGX; }
    float getMaxGY() const { return maxGY; }
    float getMaxGZ() const { return maxGZ; }

    float meanAX() const { return sumAX / count; }
    float meanAY() const { return sumAY / count; }
    float meanAZ() const { return sumAZ / count; }
    float meanGX() const { return sumGX / count; }
    float meanGY() const { return sumGY / count; }
    float meanGZ() const { return sumGZ / count; }

    float stdAX() const { return computeStd(sumAX, sumSqAX); }
    float stdAY() const { return computeStd(sumAY, sumSqAY); }
    float stdAZ() const { return computeStd(sumAZ, sumSqAZ); }
    float stdGX() const { return computeStd(sumGX, sumSqGX); }
    float stdGY() const { return computeStd(sumGY, sumSqGY); }
    float stdGZ() const { return computeStd(sumGZ, sumSqGZ); }

    float magAccMean() const  { return magAccSum / count; }
    float magGyroMean() const { return magGyroSum / count; }

private:
    DataStatistics() { reset(); }

    uint32_t count;

    float sumAX, sumAY, sumAZ;
    float sumGX, sumGY, sumGZ;

    float sumSqAX, sumSqAY, sumSqAZ;
    float sumSqGX, sumSqGY, sumSqGZ;

    float minAX, minAY, minAZ;
    float minGX, minGY, minGZ;

    float maxAX, maxAY, maxAZ;
    float maxGX, maxGY, maxGZ;

    float magAccSum;
    float magGyroSum;

    // Helpers
    void resetAxis(float &a, float &b, float &c, float &d, float &e, float &f) {
        a = b = c = d = e = f = 0.0f;
    }

    void initMinMax() {
        const float INF = 1e9;
        minAX = minAY = minAZ = minGX = minGY = minGZ =  INF;
        maxAX = maxAY = maxAZ = maxGX = maxGY = maxGZ = -INF;
    }

    inline void updateMinMax(float v, float &m) {
        if (v < m) m = v;
    }

    inline void updateMax(float v, float &m) {
        if (v > m) m = v;
    }

    float computeStd(float sum, float sumSq) const {
        float mean = sum / count;
        float meanSq = sumSq / count;
        float var = meanSq - mean * mean;
        return (var > 0 ? sqrtf(var) : 0.0f);
    }
};
