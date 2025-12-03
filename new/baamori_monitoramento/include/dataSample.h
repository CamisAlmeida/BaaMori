#pragma once
#include <stdint.h>

class dataSample
{
private:
    int16_t accX, accY, accZ;
    int16_t gyroX, gyroY, gyroZ;

public:

    // 🔥 Construtor default NECESSÁRIO
    dataSample()
        : accX(0), accY(0), accZ(0), gyroX(0), gyroY(0), gyroZ(0) {}

    dataSample(int16_t aX, int16_t aY, int16_t aZ,
               int16_t gX, int16_t gY, int16_t gZ)
        : accX(aX), accY(aY), accZ(aZ),
          gyroX(gX), gyroY(gY), gyroZ(gZ) {}

    int16_t getAccX() const { return accX; }
    int16_t getAccY() const { return accY; }
    int16_t getAccZ() const { return accZ; }

    int16_t getGyroX() const { return gyroX; }
    int16_t getGyroY() const { return gyroY; }
    int16_t getGyroZ() const { return gyroZ; }
};
