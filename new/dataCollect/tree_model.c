int predict_fall_from_features(double features[]) {
    if (features[20] <= 3535.576660156250) {
        if (features[9] <= -968.000000000000) {
            return 0;
        } else {
            return 0;
        }
    } else {
        if (features[3] <= -912.000000000000) {
            if (features[18] <= 3708.223632812500) {
                return 1;
            } else {
                return 1;
            }
        } else {
            return 0;
        }
    }
}
