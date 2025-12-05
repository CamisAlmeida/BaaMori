int predict_fall_from_features(double features[]) {
    if (features[4] <= 3964.802124023438) {
        if (features[15] <= 19.265387535095) {
            if (features[13] <= -1248.459960937500) {
                return 0;
            } else {
                return 0;
            }
        } else {
            if (features[0] <= 1981.239990234375) {
                if (features[15] <= 21.009377479553) {
                    if (features[16] <= 2124.553161621094) {
                        return 0;
                    } else {
                        return 0;
                    }
                } else {
                    return 0;
                }
            } else {
                return 1;
            }
        }
    } else {
        if (features[9] <= 5292.000000000000) {
            return 0;
        } else {
            if (features[16] <= 6654.609375000000) {
                if (features[3] <= 4662.233642578125) {
                    return 1;
                } else {
                    return 1;
                }
            } else {
                return 1;
            }
        }
    }
}
