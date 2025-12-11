import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import _tree
import json


# Função para exportar uma árvore individual para C
def export_tree_to_c(tree, tree_id):
    tree_ = tree.tree_
    features = tree_.feature
    thresholds = tree_.threshold
    children_left = tree_.children_left
    children_right = tree_.children_right
    values = tree_.value[:, 0, :]  # classes

    lines = []
    lines.append(f"int predict_tree_{tree_id}(float *x) {{")

    def recurse(node, depth):
        indent = "  " * depth
        if children_left[node] == _tree.TREE_LEAF:
            cls = np.argmax(values[node])
            lines.append(f"{indent}return {cls};")
        else:
            feat = features[node]
            thr = thresholds[node]

            lines.append(f"{indent}if (x[{feat}] <= {thr}f) {{")
            recurse(children_left[node], depth + 1)
            lines.append(f"{indent}}} else {{")
            recurse(children_right[node], depth + 1)
            lines.append(f"{indent}}}")

    recurse(0, 1)
    lines.append("}\n")
    return "\n".join(lines)


# Função para exportar a RandomForest inteira
def export_forest_to_c(clf):
    header = "// ===== RandomForest exportada automaticamente =====\n\n"

    trees_c = []
    for i, estimator in enumerate(clf.estimators_):
        trees_c.append(export_tree_to_c(estimator, i))

    # Função principal (votação)
    main_fn = """
int predict_forest(float *x) {
    int votes[2] = {0, 0};
"""

    for i in range(len(clf.estimators_)):
        main_fn += f"    votes[predict_tree_{i}(x)]++;\n"

    main_fn += """
    return (votes[1] > votes[0]) ? 1 : 0;
}
"""

    return header + "\n".join(trees_c) + main_fn


# THRESHOLD DINÂMICO
def compute_dynamic_threshold(df):
    """
    Ajusta o threshold automático baseado na magnitude média das quedas.
    Assume que a feature 'acc_mag' existe.
    """

    mag_fall = df[df["label"] == 1]['acc_mag'].mean()
    mag_normal = df[df["label"] == 0]['acc_mag'].mean()

    # Fórmula simples + robusta:
    threshold = (mag_fall + mag_normal) / 2
    return threshold


# CARREGAR DADOS E TREINAR MODELO
df = pd.read_csv("new\dataCollect\dataset_balanceado.csv")

X = df.drop(columns=["label"]).values
y = df["label"].values

# Calcular threshold dinâmico
dynamic_threshold = compute_dynamic_threshold(df)

# Divisão treino/teste
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# TREINAR RANDOM FOREST
clf = RandomForestClassifier(
    n_estimators=10,
    max_depth=6,
    min_samples_leaf=3,
    random_state=42
)

clf.fit(X_train, y_train)

print("Acurácia:", clf.score(X_test, y_test))

# EXPORTAR PARA C
c_code = export_forest_to_c(clf)

with open("random_forest_exported.c", "w") as f:
    f.write(c_code)

# EXPORTAR THRESHOLD DINÂMICO
with open("dynamic_threshold.h", "w") as f:
    f.write(f"#define FALL_DYNAMIC_THRESHOLD {dynamic_threshold}f\n")

print("Exportação concluída!")
