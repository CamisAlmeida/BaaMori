#pragma once
#include <stdint.h>
#include <math.h>

#include "dataStatistics.h"

class decisionTree {
public:
    decisionTree() : isFall(false) {}

    bool getIsFall() const { return isFall; }

    void resetFall() { isFall = false; }

    void checkFall(const DataStatistics &statistic) {
        // Se ainda não há amostras, não decide
        if (statistic.getCount() == 0) {
            isFall = false;
            return;
        }

        int pred;

        if (statistic.getMaxAZ() <= 3535.576660156250) {
            if (statistic.meanGX() <= -1035.839965820312) {
                pred = 0;
            } else {
                pred = 0;
            }
        } else {
            if ( statistic.meanAX() <= -912.0) {
                if (statistic.magAccMean() <= 3265.449340820312) {
                    pred = 1;
                } else {
                    pred = 1;
                }
            } else {
                pred = 0;
            }
        }

        // Atualiza a flag interna
        isFall = (pred == 1);
    }

private:
    bool isFall;
    
};
