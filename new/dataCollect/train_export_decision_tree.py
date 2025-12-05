"""
train_export_decision_tree.py
"""

import os
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier, _tree, export_text
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import classification_report, confusion_matrix
from sklearn.ensemble import RandomForestClassifier
import joblib

# CONFIGURAÇÕES
BASE = "new/dataCollect/"

normal_csv = BASE + "normal_2025-11-25_15-16-55_cotidiano.csv"

queda_files = [
    BASE + "queda_2025-11-25_16-52-59_traz.csv",
    BASE + "queda_2025-11-25_16-58-06_frente1.csv",
    BASE + "queda_2025-11-25_17-06-41-frente2.csv",
    BASE + "queda_2025-11-25_17-10-20_lado.csv",
]

timestamp_cols = ["timestamp_inicio", "timestamp_fim", "timestamp", "time"]
drop_cols = ["fall_id", "n_samples"]
label_col = "classe"

# ÁRVORE GRANDE
MAX_DEPTH = 10
MIN_SAMPLES_LEAF = 4


# =============================
# FUNÇÕES AUXILIARES
# =============================
def load_csv(path):
    print("Lendo:", path)
    return pd.read_csv(path)


def standardize_columns(df):
    df = df.rename(columns=lambda s: s.strip() if isinstance(s, str) else s)
    return df


# =============================
# CARREGAR E UNIFICAR DATASETS
# =============================

print("\nCarregando arquivos...")

df_normal = load_csv(normal_csv)
df_normal[label_col] = 0  # normal

queda_list = []
for q in queda_files:
    df_q = load_csv(q)
    df_q[label_col] = 1  # queda
    queda_list.append(df_q)

df_queda = pd.concat(queda_list, ignore_index=True)

df = pd.concat([df_normal, df_queda], ignore_index=True)

df = standardize_columns(df)


# =============================
# REMOVER COLUNAS NÃO NUMÉRICAS
# =============================
for col in timestamp_cols + drop_cols:
    if col in df.columns:
        df = df.drop(columns=[col])


# =============================
# DEFINIR FEATURES
# =============================
feature_names = [
    "mean_ax", "mean_ay", "mean_az",
    "std_ax", "std_ay", "std_az",
    "min_ax", "min_ay", "min_az",
    "max_ax", "max_ay", "max_az",

    "mean_gx", "mean_gy", "mean_gz",
    "std_gx", "std_gy", "std_gz",
    "min_gx", "min_gy", "min_gz",
    "max_gx", "max_gy", "max_gz",
]

# Garantir apenas as colunas que existem
feature_names = [f for f in feature_names if f in df.columns]

X = df[feature_names].values
y = df[label_col].values

print(f"\nTotal de amostras: {len(df)}")
print(f"Features usadas ({len(feature_names)}): {feature_names}")


# =============================
# TREINO / TESTE
# =============================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.25, random_state=42, stratify=y
)

# ===== RANDOM FOREST ==========

clf = RandomForestClassifier(
    n_estimators=100,
    max_depth=10,
    min_samples_leaf=8,
    class_weight="balanced",
    max_features="sqrt",
    random_state=42
)

clf.fit(X_train, y_train)


# =============================
# AVALIAÇÃO
# =============================
y_pred = clf.predict(X_test)

report_text = classification_report(y_test, y_pred)
cm_text = str(confusion_matrix(y_test, y_pred))

print("\n===== CLASSIFICATION REPORT =====\n")
print(report_text)

print("\n===== MATRIZ DE CONFUSÃO =====\n")
print(cm_text)

scores = cross_val_score(clf, X, y, cv=5)
cv_text = "\nCross-val accuracy (5-fold): " + str(scores) + " Média: " + str(scores.mean())

print(cv_text)

# SALVAR MÉTRICAS
with open(BASE + "metricas_decision_tree.txt", "w") as f:
    f.write("===== CLASSIFICATION REPORT =====\n")
    f.write(report_text)
    f.write("\n\n===== MATRIZ DE CONFUSÃO =====\n")
    f.write(cm_text)
    f.write("\n\n" + cv_text)

print("\nMétricas salvas em:", BASE + "metricas_decision_tree.txt")


# =============================
# IMPORTÂNCIA DAS FEATURES
# =============================
feat_imp = sorted(zip(feature_names, clf.feature_importances_), key=lambda x: x[1], reverse=True)
print("\n===== IMPORTÂNCIAS =====")
for f, imp in feat_imp:
    print(f"{f}: {imp:.4f}")


# =============================
# SALVAR MODELO
# =============================
joblib.dump(clf, BASE + "random_forest_model.joblib")
print("\nModelo salvo em:", BASE + "random_forest_model.joblib")


# ============================================
#      EXTRAIR A MELHOR ÁRVORE DA FLORESTA
# ============================================

best_tree_index = np.argmax([estimator.tree_.node_count for estimator in clf.estimators_])
best_tree = clf.estimators_[best_tree_index]

print("\nUsando a árvore simplificada número:", best_tree_index)


# ===========================================================
#     FUNÇÃO: CONVERTER A MELHOR ÁRVORE PARA C
# ===========================================================
def tree_to_c(tree, feature_names, func_name="predict_fall_from_features"):

    t = tree.tree_

    def generate(node, depth):
        indent = "    " * depth
        if t.feature[node] != _tree.TREE_UNDEFINED:
            feat = feature_names[t.feature[node]]
            thr = t.threshold[node]
            left = t.children_left[node]
            right = t.children_right[node]

            return (
                f"{indent}if (features[{t.feature[node]}] <= {thr:.12f}) {{\n" +
                generate(left, depth + 1) +
                f"{indent}}} else {{\n" +
                generate(right, depth + 1) +
                f"{indent}}}\n"
            )
        else:
            cid = int(np.argmax(t.value[node][0]))
            return f"{indent}return {cid};\n"

    header = f"int {func_name}(double features[]) {{\n"
    body = generate(0, 1)
    footer = "}\n"
    return header + body + footer


c_code = tree_to_c(best_tree, feature_names)

with open(BASE + "tree_model.c", "w") as f:
    f.write(c_code)

print("Código C salvo em:", BASE + "tree_model.c")
