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

    int predict_tree_0(const DataStatistics &s) {
        // x[6] -> max_ax, x[20] -> std_az, x[16] -> mean_gy, x[11] -> max_gz
        if (s.getMaxAX() <= 12948.0f) {
            if (s.stdAZ() <= 5236.853271f) {
                if (s.stdAZ() <= 4243.042725f) {
                    return 0;
                } else {
                    return 0;
                }
            } else {
                return 1;
            }
        } else {
            if (s.meanGY() <= -141.865501f) {
                if (s.getMaxGZ() <= 32479.0f) {
                    return 1;
                } else {
                    return 1;
                }
            } else {
                return 1;
            }
        }
    }

    int predict_tree_1(const DataStatistics &s) {
        // x[10] -> max_gy, x[20] -> std_az, x[12] -> mean_ax
        if (s.getMaxGY() <= 11066.5f) {
            if (s.stdAZ() <= 3516.118896f) {
                return 0;
            } else {
                return 0;
            }
        } else {
            if (s.meanAX() <= -3776.125244f) {
                return 0;
            } else {
                if (s.stdAZ() <= 4879.684082f) {
                    return 1;
                } else {
                    return 1;
                }
            }
        }
    }

    int predict_tree_2(const DataStatistics &s) {
        // x[3] -> min_gx, x[6] -> max_ax, x[2] -> min_az
        if (s.getMinGX() <= -1128.0f) {
            return 1;
        } else {
            if (s.getMaxAX() <= 15122.0f) {
                if (s.getMinAZ() <= -10976.0f) {
                    return 0;
                } else {
                    return 0;
                }
            } else {
                return 1;
            }
        }
    }

    int predict_tree_3(const DataStatistics &s) {
        // x[6] -> max_ax, x[3] -> min_gx, x[17] -> mean_gz
        if (s.getMaxAX() <= 11816.0f) {
            if (s.getMinGX() <= -1144.0f) {
                return 1;
            } else {
                if (s.meanGZ() <= 1880.407959f) {
                    return 0;
                } else {
                    return 0;
                }
            }
        } else {
            if (s.getMinGX() <= -1024.0f) {
                return 1;
            } else {
                return 1;
            }
        }
    }

    int predict_tree_4(const DataStatistics &s) {
        // x[20] -> std_az, x[9] -> max_gx, x[5] -> min_gz, x[7] -> max_ay
        if (s.stdAZ() <= 4661.826904f) {
            if (s.getMaxGX() <= -984.0f) {
                return 0;
            } else {
                if (s.getMinGZ() <= -17972.0f) {
                    return 0;
                } else {
                    return 0;
                }
            }
        } else {
            if (s.getMaxAY() <= -4390.0f) {
                return 1;
            } else {
                return 1;
            }
        }
    }

    int predict_tree_5(const DataStatistics &s) {
        // x[15] -> mean_gx, x[5] -> min_gz, x[10] -> max_gy
        if (s.meanGX() <= -1060.998657f) {
            return 1;
        } else {
            if (s.getMinGZ() <= -25214.0f) {
                return 1;
            } else {
                if (s.getMaxGY() <= 14105.0f) {
                    return 0;
                } else {
                    return 0;
                }
            }
        }
    }

    int predict_tree_6(const DataStatistics &s) {
        // x[6] -> max_ax, x[10] -> max_gy, x[17] -> mean_gz
        if (s.getMaxAX() <= 12116.0f) {
            if (s.getMaxGY() <= 16357.0f) {
                return 0;
            } else {
                return 1;
            }
        } else {
            if (s.meanGZ() <= 2908.596191f) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    int predict_tree_7(const DataStatistics &s) {
        // x[20] -> std_az, x[6] -> max_ax
        if (s.stdAZ() <= 4030.362793f) {
            if (s.getMaxAX() <= 10646.0f) {
                return 0;
            } else {
                return 0;
            }
        } else {
            if (s.getMaxAX() <= 9970.0f) {
                return 1;
            } else {
                return 1;
            }
        }
    }

    int predict_tree_8(const DataStatistics &s) {
        // x[6] -> max_ax, x[18] -> std_ax, x[20] -> std_az, x[19] -> std_ay
        if (s.getMaxAX() <= 10112.0f) {
            if (s.stdAX() <= 4735.433838f) {
                if (s.stdAZ() <= 3574.100708f) {
                    return 0;
                } else {
                    return 0;
                }
            } else {
                return 1;
            }
        } else {
            if (s.stdAY() <= 3814.042725f) {
                return 1;
            } else {
                return 1;
            }
        }
    }

    int predict_tree_9(const DataStatistics &s) {
        // x[20] -> std_az
        if (s.stdAZ() <= 4027.881348f) {
            return 0;
        } else {
            return 1;
        }
    }

    // Votação da floresta
    int predict_forest(const DataStatistics &s) {
        int votes0 = 0;
        int votes1 = 0;

        auto vote = [&](int v){ if (v == 1) votes1++; else votes0++; };

        vote(predict_tree_0(s));
        vote(predict_tree_1(s));
        vote(predict_tree_2(s));
        vote(predict_tree_3(s));
        vote(predict_tree_4(s));
        vote(predict_tree_5(s));
        vote(predict_tree_6(s));
        vote(predict_tree_7(s));
        vote(predict_tree_8(s));
        vote(predict_tree_9(s));

        return (votes1 > votes0) ? 1 : 0;
    }

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
    bool isFall;
};
