// ===== RandomForest exportada automaticamente =====

int predict_tree_0(float *x) {
  if (x[6] <= 12948.000000f) {
    if (x[20] <= 5236.853271f) {
      if (x[20] <= 4243.042725f) {
        return 0;
      } else {
        return 0;
      }
    } else {
      return 1;
    }
  } else {
    if (x[16] <= -141.865501f) {
      if (x[11] <= 32479.000000f) {
        return 1;
      } else {
        return 1;
      }
    } else {
      return 1;
    }
  }
}

int predict_tree_1(float *x) {
  if (x[10] <= 11066.500000f) {
    if (x[20] <= 3516.118896f) {
      return 0;
    } else {
      return 0;
    }
  } else {
    if (x[12] <= -3776.125244f) {
      return 0;
    } else {
      if (x[20] <= 4879.684082f) {
        return 1;
      } else {
        return 1;
      }
    }
  }
}

int predict_tree_2(float *x) {
  if (x[3] <= -1128.000000f) {
    return 1;
  } else {
    if (x[6] <= 15122.000000f) {
      if (x[2] <= -10976.000000f) {
        return 0;
      } else {
        return 0;
      }
    } else {
      return 1;
    }
  }
}

int predict_tree_3(float *x) {
  if (x[6] <= 11816.000000f) {
    if (x[3] <= -1144.000000f) {
      return 1;
    } else {
      if (x[17] <= 1880.407959f) {
        return 0;
      } else {
        return 0;
      }
    }
  } else {
    if (x[3] <= -1024.000000f) {
      return 1;
    } else {
      return 1;
    }
  }
}

int predict_tree_4(float *x) {
  if (x[20] <= 4661.826904f) {
    if (x[9] <= -984.000000f) {
      return 0;
    } else {
      if (x[5] <= -17972.000000f) {
        return 0;
      } else {
        return 0;
      }
    }
  } else {
    if (x[7] <= -4390.000000f) {
      return 1;
    } else {
      return 1;
    }
  }
}

int predict_tree_5(float *x) {
  if (x[15] <= -1060.998657f) {
    return 1;
  } else {
    if (x[5] <= -25214.000000f) {
      return 1;
    } else {
      if (x[10] <= 14105.000000f) {
        return 0;
      } else {
        return 0;
      }
    }
  }
}

int predict_tree_6(float *x) {
  if (x[6] <= 12116.000000f) {
    if (x[10] <= 16357.000000f) {
      return 0;
    } else {
      return 1;
    }
  } else {
    if (x[17] <= 2908.596191f) {
      return 1;
    } else {
      return 0;
    }
  }
}

int predict_tree_7(float *x) {
  if (x[20] <= 4030.362793f) {
    if (x[6] <= 10646.000000f) {
      return 0;
    } else {
      return 0;
    }
  } else {
    if (x[6] <= 9970.000000f) {
      return 1;
    } else {
      return 1;
    }
  }
}

int predict_tree_8(float *x) {
  if (x[6] <= 10112.000000f) {
    if (x[18] <= 4735.433838f) {
      if (x[20] <= 3574.100708f) {
        return 0;
      } else {
        return 0;
      }
    } else {
      return 1;
    }
  } else {
    if (x[19] <= 3814.042725f) {
      return 1;
    } else {
      return 1;
    }
  }
}

int predict_tree_9(float *x) {
  if (x[20] <= 4027.881348f) {
    return 0;
  } else {
    return 1;
  }
}


int predict_forest(float *x) {
    int votes[2];
    for (int i=0;i<2;++i) votes[i]=0;
    votes[predict_tree_0(x)]++;
    votes[predict_tree_1(x)]++;
    votes[predict_tree_2(x)]++;
    votes[predict_tree_3(x)]++;
    votes[predict_tree_4(x)]++;
    votes[predict_tree_5(x)]++;
    votes[predict_tree_6(x)]++;
    votes[predict_tree_7(x)]++;
    votes[predict_tree_8(x)]++;
    votes[predict_tree_9(x)]++;
    int best_idx = 0;
    for (int i=1;i<2;++i) if (votes[i] > votes[best_idx]) best_idx = i;
    int class_map[] = {0, 1};
    return class_map[best_idx];
}
