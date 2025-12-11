#pragma once
#include <stdint.h>
#include <math.h>

#include "dataStatistics.h"

class decisionTree
{
public:
    decisionTree() : isFall(false) {}

    bool getIsFall() const { return isFall; }

    void resetFall() { isFall = false; }

    void checkFall(const DataStatistics &statistic)
    {
        if (statistic.getCount() == 0)
        {
            isFall = false;
            return;
        }
        int pred;
        if (statistic.getMinGY() <= 3964.802124023438)
        {
            if (statistic.meanGX() <= 19.265387535095)
            {
                if (statistic.meanAY() <= -1248.459960937500)
                {
                    pred = 0;
                }
                else
                {
                    pred = 0;
                }
            }
            else
            {
                if (statistic.getMinAX() <= 1981.239990234375)
                {
                    if (statistic.meanGX() <= 21.009377479553)
                    {
                        if (statistic.meanGY() <= 2124.553161621094)
                        {
                            pred = 0;
                        }
                        else
                        {
                            pred = 0;
                        }
                    }
                    else
                    {
                        pred = 0;
                    }
                }
                else
                {
                    pred = 1;
                }
            }
        }
        else
        {
            if (statistic.getMaxGX() <= 5292.0)
            {
                pred = 0;
            }
            else
            {
                if (statistic.meanGY() <= 6654.609375)
                {
                    if (statistic.getMinGX() <= 4662.233642578125)
                    {
                        pred = 1;
                    }
                    else
                    {
                        pred = 1;
                    }
                }
                else
                {
                    pred = 1;
                }
            }
        }

        isFall = (pred == 1);
    }

private:
    bool isFall;
};
