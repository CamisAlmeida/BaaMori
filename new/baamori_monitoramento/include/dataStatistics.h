#pragma once
#include <stdint.h>
#include <math.h>

class DataStatistics {
public:
    DataStatistics() { reset(); }

    // Reset total
    void reset() {
        count = 0;

        // Zerar somas e somas ao quadrado
        resetAxis(sumAX, sumAY, sumAZ, sumGX, sumGY, sumGZ);
        resetAxis(sumSqAX, sumSqAY, sumSqAZ, sumSqGX, sumSqGY, sumSqGZ);

        // Inicializar min/max
        initMinMax();
        
        magAccSum = 0.0f;
        magGyroSum = 0.0f;
    }

    // Atualiza estatísticas com uma nova amostra (online)
    void update(float ax, float ay, float az, float gx, float gy, float gz) {
        count++;

        // ----- Soma -----
        sumAX += ax;  sumAY += ay;  sumAZ += az;
        sumGX += gx;  sumGY += gy;  sumGZ += gz;

        // ----- Soma dos quadrados -----
        sumSqAX += ax*ax;  sumSqAY += ay*ay;  sumSqAZ += az*az;
        sumSqGX += gx*gx;  sumSqGY += gy*gy;  sumSqGZ += gz*gz;

        // ----- Min/Max -----
        updateMinMax(ax, minAX); updateMinMax(ay, minAY); updateMinMax(az, minAZ);
        updateMinMax(gx, minGX); updateMinMax(gy, minGY); updateMinMax(gz, minGZ);

        updateMax(ax, maxAX); updateMax(ay, maxAY); updateMax(az, maxAZ);
        updateMax(gx, maxGX); updateMax(gy, maxGY); updateMax(gz, maxGZ);

        // Magnitudes
        magAccSum  += sqrtf(ax*ax + ay*ay + az*az);
        magGyroSum += sqrtf(gx*gx + gy*gy + gz*gz);
    }

    // ----- Getters -----
    uint32_t getCount() const { return count; }

    // Min
    float getMinAX() const { return minAX; }
    float getMinAY() const { return minAY; }
    float getMinAZ() const { return minAZ; }
    float getMinGX() const { return minGX; }
    float getMinGY() const { return minGY; }
    float getMinGZ() const { return minGZ; }

    // Max
    float getMaxAX() const { return maxAX; }
    float getMaxAY() const { return maxAY; }
    float getMaxAZ() const { return maxAZ; }
    float getMaxGX() const { return maxGX; }
    float getMaxGY() const { return maxGY; }
    float getMaxGZ() const { return maxGZ; }

    // Means
    float meanAX() const { return sumAX / count; }
    float meanAY() const { return sumAY / count; }
    float meanAZ() const { return sumAZ / count; }
    float meanGX() const { return sumGX / count; }
    float meanGY() const { return sumGY / count; }
    float meanGZ() const { return sumGZ / count; }

    // Standard Deviation (online)
    float stdAX() const { return computeStd(sumAX, sumSqAX); }
    float stdAY() const { return computeStd(sumAY, sumSqAY); }
    float stdAZ() const { return computeStd(sumAZ, sumSqAZ); }
    float stdGX() const { return computeStd(sumGX, sumSqGX); }
    float stdGY() const { return computeStd(sumGY, sumSqGY); }
    float stdGZ() const { return computeStd(sumGZ, sumSqGZ); }

    // Magnitude média
    float magAccMean() const  { return magAccSum / count; }
    float magGyroMean() const { return magGyroSum / count; }

private:
    uint32_t count;

    // Somatórios
    float sumAX, sumAY, sumAZ;
    float sumGX, sumGY, sumGZ;

    // Somatório dos quadrados
    float sumSqAX, sumSqAY, sumSqAZ;
    float sumSqGX, sumSqGY, sumSqGZ;

    // Min e Max
    float minAX, minAY, minAZ;
    float minGX, minGY, minGZ;

    float maxAX, maxAY, maxAZ;
    float maxGX, maxGY, maxGZ;

    // Magnitude
    float magAccSum;
    float magGyroSum;

    // --- Helpers ---
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

    // std = sqrt( E[x²] - (E[x])² )
    float computeStd(float sum, float sumSq) const {
        float mean = sum / count;
        float meanSq = sumSq / count;
        float var = meanSq - mean * mean;
        return (var > 0 ? sqrtf(var) : 0.0f);
    }
};
