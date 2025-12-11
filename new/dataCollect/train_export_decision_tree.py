import os
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import _tree
import json

# --- Funções de exportação ---

def export_tree_to_c(tree, tree_id, n_classes):
    tree_ = tree.tree_
    features = tree_.feature
    thresholds = tree_.threshold
    children_left = tree_.children_left
    children_right = tree_.children_right
    values = tree_.value[:, 0, :]  # shape: (n_nodes, n_classes)

    lines = []
    lines.append(f"int predict_tree_{tree_id}(float *x) {{")

    def recurse(node, depth):
        indent = "  " * depth
        if children_left[node] == _tree.TREE_LEAF:
            cls_index = int(np.argmax(values[node]))
            lines.append(f"{indent}return {cls_index};")
        else:
            feat = int(features[node])
            thr = float(thresholds[node])
            lines.append(f"{indent}if (x[{feat}] <= {thr:.6f}f) {{")
            recurse(children_left[node], depth + 1)
            lines.append(f"{indent}}} else {{")
            recurse(children_right[node], depth + 1)
            lines.append(f"{indent}}}")

    recurse(0, 1)
    lines.append("}\n")
    return "\n".join(lines)


def export_forest_to_c(clf):
    header = "// ===== RandomForest exportada automaticamente =====\n\n"
    n_classes = clf.n_classes_
    # criar class_map a partir de clf.classes_
    classes = [int(c) for c in clf.classes_]

    # gerar código das árvores
    trees_c = []
    for i, estimator in enumerate(clf.estimators_):
        trees_c.append(export_tree_to_c(estimator, i, n_classes))

    # Função principal (votação) - usa tamanho dinâmico de classes e class_map
    main_fn = []
    main_fn.append(f"int predict_forest(float *x) {{")
    main_fn.append(f"    int votes[{n_classes}];")
    main_fn.append(f"    for (int i=0;i<{n_classes};++i) votes[i]=0;")
    for i in range(len(clf.estimators_)):
        main_fn.append(f"    votes[predict_tree_{i}(x)]++;")
    main_fn.append(f"    int best_idx = 0;")
    main_fn.append(f"    for (int i=1;i<{n_classes};++i) if (votes[i] > votes[best_idx]) best_idx = i;")
    # class_map com os rótulos originais
    class_map_c = "{" + ", ".join(str(c) for c in classes) + "}"
    main_fn.append(f"    int class_map[] = {class_map_c};")
    main_fn.append(f"    return class_map[best_idx];")
    main_fn.append("}")

    return header + "\n".join(trees_c) + "\n\n" + "\n".join(main_fn) + "\n"


# --- THRESHOLD DINÂMICO ---
def compute_dynamic_threshold(df):
    if "mag_acc_mean" not in df.columns:
        raise KeyError("Coluna 'mag_acc_mean' não encontrada no dataframe para calcular threshold dinâmico.")
    # evita NaNs e divide por zero
    mag_fall = df[df["classe"] == 1]["mag_acc_mean"].dropna()
    mag_normal = df[df["classe"] == 0]["mag_acc_mean"].dropna()

    if len(mag_fall) == 0 or len(mag_normal) == 0:
        # fallback: usar média global ou zero
        print("Aviso: uma das classes não tem amostras para 'mag_acc_mean'. Usando média global.")
        overall = df["mag_acc_mean"].dropna()
        if overall.empty:
            return 0.0
        return float(overall.mean())

    threshold = float((mag_fall.mean() + mag_normal.mean()) / 2.0)
    return threshold


# --- MAIN: carregar dados, treinar e exportar ---
csv_path = os.path.join("new", "dataCollect", "dataset_balanceado.csv")
print("Tentando carregar:", csv_path)
df = pd.read_csv(csv_path)

# Diagnostics
print("DataFrame shape:", df.shape)
print("Colunas:", df.columns.tolist())
print("Primeiras linhas:\n", df.head())

# Verificações essenciais
if "classe" not in df.columns:
    raise KeyError("Coluna 'classe' não encontrada no CSV. Verifique o nome exato (sensível a maiúsculas).")
if df["classe"].isnull().any():
    print("Aviso: existem classes NaN. Removendo linhas com classe NaN.")
    df = df[df["classe"].notnull()]

# Separar X e y (garantindo usar nomes corretos)
X = df.drop(columns=["classe"]).values
y = df["classe"].values.astype(int)  # assumindo rótulos inteiros (ajuste se não)

# Calcular threshold dinâmico (opcional)
try:
    dynamic_threshold = compute_dynamic_threshold(df)
except KeyError as e:
    print("compute_dynamic_threshold: ", e)
    dynamic_threshold = 0.0

# Divisão treino/teste
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y if len(np.unique(y))>1 else None
)

# Treinar RandomForest
clf = RandomForestClassifier(
    n_estimators=10,
    max_depth=15,
    min_samples_leaf=4,
    random_state=42
)

clf.fit(X_train, y_train)

print("Acurácia:", clf.score(X_test, y_test))

# Exportar para C
c_code = export_forest_to_c(clf)
with open("random_forest_exported.c", "w") as f:
    f.write(c_code)

# Exportar threshold dinâmico em header
with open("dynamic_threshold.h", "w") as f:
    f.write(f"#define FALL_DYNAMIC_THRESHOLD {dynamic_threshold:.6f}f\n")

print("Exportação concluída!")
