"""
train_export_decision_tree.py
"""

import os
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier, _tree
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import classification_report, confusion_matrix
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
drop_cols = ["fall_id", "n_samples"]   # <-- REMOVIDO AQUI
label_col = "classe"

MAX_DEPTH = 6
MIN_SAMPLES_LEAF = 6

# FUNÇÕES AUXILIARES

def load_csv(path):
    print("Lendo:", path)
    return pd.read_csv(path)

def standardize_columns(df):
    df = df.rename(columns=lambda s: s.strip() if isinstance(s, str) else s)
    return df

# CARREGAR QUEDAS

queda_dfs = []
for f in queda_files:
    if not os.path.isfile(f):
        raise FileNotFoundError(f"Arquivo de queda não encontrado: {f}")
    dfq = load_csv(f)
    dfq = standardize_columns(dfq)
    queda_dfs.append(dfq)

queda_all = pd.concat(queda_dfs, ignore_index=True)
print("Total quedas (linhas):", len(queda_all))

# CARREGAR NORMAIS

if not os.path.isfile(normal_csv):
    raise FileNotFoundError(f"Arquivo normal não encontrado: {normal_csv}")

df_norm = load_csv(normal_csv)
df_norm = standardize_columns(df_norm)
print("Total normais (linhas):", len(df_norm))

# REMOVER COLUNAS DESNECESSÁRIAS

cols_to_drop = [c for c in df_norm.columns if any(t in c.lower() for t in timestamp_cols)]
cols_to_drop += drop_cols
cols_to_drop = list(set(cols_to_drop))

print("Colunas a remover:", cols_to_drop)

queda_all = queda_all.drop(columns=[c for c in cols_to_drop if c in queda_all.columns], errors="ignore")
df_norm   = df_norm.drop(columns=[c for c in cols_to_drop if c in df_norm.columns], errors="ignore")

# GARANTIR LABEL

if label_col not in queda_all.columns:
    queda_all[label_col] = 1
if label_col not in df_norm.columns:
    df_norm[label_col] = 0

# ALINHAR COLUNAS

common_cols = [c for c in queda_all.columns if c in df_norm.columns]

if label_col in common_cols:
    common_cols = [c for c in common_cols if c != label_col] + [label_col]

print("Colunas usadas:", common_cols)

queda_all = queda_all[common_cols].copy()
df_norm   = df_norm[common_cols].copy()

# BALANCEAR DATASET

N_quedas = len(queda_all)
print("Número de quedas:", N_quedas)

df_norm_sampled = df_norm.sample(n=N_quedas, random_state=42).reset_index(drop=True)

df_bal = pd.concat([queda_all, df_norm_sampled], ignore_index=True)\
           .sample(frac=1, random_state=42).reset_index(drop=True)

print("Dataset balanceado tamanho:", len(df_bal))

# salvar dataset balanceado
df_bal.to_csv(BASE + "dataset_balanceado.csv", index=False)
print("Dataset balanceado salvo em:", BASE + "dataset_balanceado.csv")


# PREPARAR X E y

X = df_bal.drop(columns=[label_col])
y = df_bal[label_col].astype(int)

for c in X.columns:
    if not np.issubdtype(X[c].dtype, np.number):
        X[c] = pd.to_numeric(X[c], errors="coerce")

X = X.fillna(X.mean())

feature_names = list(X.columns)
print("Features:", feature_names)

# TREINO / TESTE

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.25, random_state=42, stratify=y
)

clf = DecisionTreeClassifier(
    max_depth=MAX_DEPTH,
    min_samples_leaf=MIN_SAMPLES_LEAF,
    random_state=42
)
clf.fit(X_train, y_train)

# AVALIAÇÃO

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

# IMPORTÂNCIAS

feat_imp = sorted(zip(feature_names, clf.feature_importances_), key=lambda x: x[1], reverse=True)

print("\n===== IMPORTÂNCIAS =====")
for f, imp in feat_imp:
    print(f"{f}: {imp:.4f}")

# SALVAR MODELO

joblib.dump(clf, BASE + "decision_tree_model.joblib")
print("\nModelo salvo em:", BASE + "decision_tree_model.joblib")

# EXPORTAR REGRAS

def tree_to_rules(clf, feature_names):
    tree = clf.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree.feature
    ]

    paths = []

    def recurse(node, path):
        if tree.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            thr = tree.threshold[node]
            recurse(tree.children_left[node],  path + [f"({name} <= {thr:.5f})"])
            recurse(tree.children_right[node], path + [f"({name} > {thr:.5f})"])
        else:
            cid = int(np.argmax(tree.value[node][0]))
            paths.append(" AND ".join(path) + f" => class={cid}")

    recurse(0, [])
    return "\n".join(paths)


rules = tree_to_rules(clf, feature_names)

with open(BASE + "rules_decision_tree.txt", "w") as f:
    f.write(rules)

print("\nRegras salvas em:", BASE + "rules_decision_tree.txt")


# EXPORTAR ÁRVORE PARA C

def tree_to_c(clf, feature_names, func_name="predict_fall_from_features"):
    tree = clf.tree_

    def generate(node, depth):
        indent = "    " * depth
        if tree.feature[node] != _tree.TREE_UNDEFINED:
            feat = feature_names[tree.feature[node]]
            thr = tree.threshold[node]
            left = tree.children_left[node]
            right = tree.children_right[node]

            return (
                f"{indent}if (features[{tree.feature[node]}] <= {thr:.12f}) {{\n" +
                generate(left, depth + 1) +
                f"{indent}}} else {{\n" +
                generate(right, depth + 1) +
                f"{indent}}}\n"
            )
        else:
            cid = int(np.argmax(tree.value[node][0]))
            return f"{indent}return {cid};\n"

    header = f"int {func_name}(double features[]) {{\n"
    body = generate(0, 1)
    footer = "}\n"

    return header + body + footer


c_code = tree_to_c(clf, feature_names)

with open(BASE + "tree_model.c", "w") as f:
    f.write(c_code)

print("Código C salvo em:", BASE + "tree_model.c")

print("Arquivos gerados:")
print(" - dataset_balanceado.csv")
print(" - metricas_decision_tree.txt")
print(" - decision_tree_model.joblib")
print(" - rules_decision_tree.txt")
print(" - tree_model.c")
