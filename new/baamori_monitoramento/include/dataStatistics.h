#pragma once
#include <stdint.h>

class DataStatistics {
public:
    DataStatistics() {
        reset();
    }

    void reset() {
        count = 0;
        sum = 0.0f;

        minVal = 999999.0f;
        maxVal = -999999.0f;

        lastValue = 0.0f;
        hasLast = false;

        deltaSum = 0.0f;
        deltaMin = 999999.0f;
        deltaMax = -999999.0f;
    }

    // Atualiza estatísticas com uma nova amostra
    void update(float x) {
        // Media, Min, Max
        sum += x;
        count++;

        if (x < minVal) minVal = x;
        if (x > maxVal) maxVal = x;

        // ===== DELTAS =====
        if (hasLast) {
            float dx = x - lastValue;

            deltaSum += dx;
            if (dx < deltaMin) deltaMin = dx;
            if (dx > deltaMax) deltaMax = dx;
        }

        lastValue = x;
        hasLast = true;
    }

    // getters
    float mean() const {
        return (count > 0 ? sum / count : 0.0f);
    }

    float min() const { return minVal; }
    float max() const { return maxVal; }

    float deltaMean() const {
        return (count > 1 ? deltaSum / (count - 1) : 0.0f);
    }

    float getDeltaMin() const { return deltaMin; }
    float getDeltaMax() const { return deltaMax; }

private:
    uint32_t count;
    float sum;

    float minVal;
    float maxVal;

    // Para calcular deltas
    float lastValue;
    bool hasLast;

    float deltaSum;
    float deltaMin;
    float deltaMax;
};
