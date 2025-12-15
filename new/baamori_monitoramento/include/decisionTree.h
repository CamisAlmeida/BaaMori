#pragma once
#include <stdint.h>
#include <math.h>
#include "dataStatistics.h"

class decisionTree
{
public:
    // 
    static decisionTree& getInstance()
    {
        static decisionTree instance;
        return instance;
    }

    decisionTree(const decisionTree&) = delete;
    decisionTree& operator=(const decisionTree&) = delete;

    bool getIsFall() const { return isFall; }
    void resetFall() { isFall = false; }

    bool checkFall(const DataStatistics &statistic)
    {
        if (statistic.getCount() == 0) {
            isFall = false;
            return false;
        }

        int pred = predict_forest(statistic);
        isFall = (pred == 1);
        return isFall;
    }

private:
    // 🔒 Construtor privado
    decisionTree() : isFall(false) {}

    bool isFall;

    /* ======= ÁRVORES ======= */

    int predict_tree_0(const DataStatistics &s) {
        if (s.getMaxAX() <= 12948.0f) {
            if (s.stdAZ() <= 5236.853271f) return 0;
            else return 1;
        } else {
            return 1;
        }
    }

    int predict_tree_1(const DataStatistics &s) {
        if (s.getMaxGY() <= 11066.5f) return 0;
        else {
            if (s.meanAX() <= -3776.125244f) return 0;
            else return 1;
        }
    }

    int predict_tree_2(const DataStatistics &s) {
        if (s.getMinGX() <= -1128.0f) return 1;
        else {
            if (s.getMaxAX() <= 15122.0f) return 0;
            else return 1;
        }
    }

    int predict_tree_3(const DataStatistics &s) {
        if (s.getMaxAX() <= 11816.0f) {
            if (s.getMinGX() <= -1144.0f) return 1;
            else return 0;
        } else return 1;
    }

    int predict_tree_4(const DataStatistics &s) {
        if (s.stdAZ() <= 4661.826904f) return 0;
        else return 1;
    }

    int predict_tree_5(const DataStatistics &s) {
        if (s.meanGX() <= -1060.998657f) return 1;
        else {
            if (s.getMinGZ() <= -25214.0f) return 1;
            else return 0;
        }
    }

    int predict_tree_6(const DataStatistics &s) {
        if (s.getMaxAX() <= 12116.0f) {
            if (s.getMaxGY() <= 16357.0f) return 0;
            else return 1;
        } else {
            if (s.meanGZ() <= 2908.596191f) return 1;
            else return 0;
        }
    }

    int predict_tree_7(const DataStatistics &s) {
        if (s.stdAZ() <= 4030.362793f) return 0;
        else return 1;
    }

    int predict_tree_8(const DataStatistics &s) {
        if (s.getMaxAX() <= 10112.0f) {
            if (s.stdAX() <= 4735.433838f) return 0;
            else return 1;
        } else return 1;
    }

    int predict_tree_9(const DataStatistics &s) {
        return (s.stdAZ() <= 4027.881348f) ? 0 : 1;
    }

    int predict_forest(const DataStatistics &s)
    {
        int votes1 = 0;

        votes1 += predict_tree_0(s);
        votes1 += predict_tree_1(s);
        votes1 += predict_tree_2(s);
        votes1 += predict_tree_3(s);
        votes1 += predict_tree_4(s);
        votes1 += predict_tree_5(s);
        votes1 += predict_tree_6(s);
        votes1 += predict_tree_7(s);
        votes1 += predict_tree_8(s);
        votes1 += predict_tree_9(s);

        return (votes1 >= 5) ? 1 : 0;
    }
};
