import argparse
from pathlib import Path

import joblib
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report
from sklearn.model_selection import train_test_split
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler

from export import export_binary_model_to_ini, export_softmax_model_to_ini, unify_ini_files
from load_data import load_data

LABEL_COLUMN = "Label"
DROP_PREFIXES = ("Actor", "Id_")
DROP_SUFFIXES = ("_Prediction",)
RANDOM_STATE = 42


def _is_drop_column(col: str) -> bool:
    return col.startswith(DROP_PREFIXES) or col.endswith(DROP_SUFFIXES)


def _is_interaction_feature(col: str) -> bool:
    if col == LABEL_COLUMN:
        return False
    if _is_drop_column(col):
        return False
    return "_" in col


def _drop_non_features(df):
    drop_cols = [col for col in df.columns if _is_drop_column(col)]
    return df.drop(columns=drop_cols)


def _get_feature_columns(df, interaction: str) -> list[str]:
    prefix = f"{interaction}_"
    return [col for col in df.columns if col.startswith(prefix)]


def _build_pipeline(max_iter: int, solver: str = "lbfgs") -> Pipeline:
    return Pipeline([
        ("scaler", StandardScaler()),
        ("clf", LogisticRegression(
            solver=solver,
            max_iter=max_iter,
            class_weight="balanced",
            random_state=RANDOM_STATE,
        )),
    ])


def get_interactions(df) -> set[str]:
    return {col.split("_", 1)[0] for col in df.columns if _is_interaction_feature(col)}


def _build_binary_target(label_series, interaction: str):
    normalized = label_series.astype(str).str.strip()
    return (normalized == interaction).astype(int)


def train_binary_model(df, cluster: str, out_path: str | Path):
    df = _drop_non_features(df)

    out_path = Path(out_path)
    out_path.mkdir(parents=True, exist_ok=True)

    models = {}

    interactions = get_interactions(df)
    for interaction in interactions:
        y = _build_binary_target(df[LABEL_COLUMN], interaction)

        # Only use that type's features
        feature_cols = _get_feature_columns(df, interaction)
        if not feature_cols:
            print(f"Skipping {interaction}: no feature columns found.")
            continue
        if y.nunique() < 2:
            print(f"Skipping {interaction}: label has a single class.")
            continue

        X = df[feature_cols]

        X_train, X_test, y_train, y_test = train_test_split(
            X, y, test_size=0.2, stratify=y, random_state=RANDOM_STATE
        )

        model = _build_pipeline(max_iter=1000)

        model.fit(X_train, y_train)

        print(f"\n=== {interaction} Binary Model ===")
        print(classification_report(y_test, model.predict(X_test)))

        joblib.dump(model, out_path / f"{interaction.lower()}_binary_{cluster}.pkl")
        models[interaction] = model

    return models


def train_softmax_model(df, cluster: str, out_path: str | Path):
    df = _drop_non_features(df)

    y = df[LABEL_COLUMN]
    X = df.drop(columns=[LABEL_COLUMN])

    if y.nunique() < 2:
        print("Skipping softmax: only one class present.")
        return None

    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, stratify=y, random_state=RANDOM_STATE
    )

    model = _build_pipeline(max_iter=2000)

    model.fit(X_train, y_train)

    print(f"\n=== Softmax Model ({cluster}) ===")
    print(classification_report(y_test, model.predict(X_test)))

    out_path = Path(out_path)
    out_path.mkdir(parents=True, exist_ok=True)
    joblib.dump(model, out_path / f"softmax_{cluster.lower()}.pkl")

    return model

def _run_training(mods_path: str | Path = None, out_dir: str = None) -> Path:
    if out_dir is None:
        out_dir = Path(__file__).resolve().parent / "out"

    out_dir = Path(out_dir)
    table_path = out_dir / "interactions"
    model_path = out_dir / "models"

    df_map = load_data(mods_path, table_path)
    model_path.mkdir(parents=True, exist_ok=True)

    for cluster, df in df_map.items():
        interaction_count = len(get_interactions(df))
        print(f"\nTraining models for cluster: {cluster}, interactions: {interaction_count}")
        if interaction_count == 0:
            print(f"No interactions found for cluster {cluster}, skipping.")
        elif interaction_count == 1:
            models = train_binary_model(df, cluster, model_path)
            for interaction, model in models.items():
                print(f"Exporting {interaction} model to INI...")
                export_binary_model_to_ini(model, interaction, model_path / f"{interaction.lower()}.ini")
        else:
            model = train_softmax_model(df, cluster, model_path)
            if model is not None:
                export_softmax_model_to_ini(model, model_path / f"softmax_{cluster.lower()}.ini")

    return model_path


def _build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Train interaction models and export INI weights.")
    parser.add_argument(
        "--mods-path",
        help="Root mods folder containing SKSE/SexLab/ModelData/*.csv files (falls back to env).",
    )
    parser.add_argument(
        "--out-dir",
        default=None,
        help="Output directory for CSVs and models (default: scripts/ML/out).",
    )
    parser.add_argument(
        "--ini-path",
        default=None,
        help="Output for the updated INI file (default: dist/SKSE/SexLab/LinearModel.ini).",
    )
    return parser

if __name__ == "__main__":
    parser = _build_arg_parser()
    args = parser.parse_args()

    out_dir = Path(args.out_dir) if args.out_dir else None
    model_path = _run_training(mods_path=args.mods_path, out_dir=out_dir)

    ini_path = Path(args.ini_path) if args.ini_path else Path("dist/SKSE/SexLab/LinearModel.ini")
    unify_ini_files(model_path, ini_path)
