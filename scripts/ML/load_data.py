import os
from collections import defaultdict
from pathlib import Path

import pandas as pd

ENV_VAR_NAME = "XSE_TES5_MODS_PATH"


def _load_env_var_from_file(env_path: Path, key: str) -> str | None:
    if not env_path.exists():
        return None

    for line in env_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if "=" not in stripped:
            continue
        name, value = stripped.split("=", 1)
        if name.strip() == key:
            return value.strip() or None
    return None


def _resolve_mods_path(in_path: str | None) -> Path:
    if in_path:
        return Path(in_path)

    env_path = Path(__file__).resolve().parents[2] / ".env"
    from_file = _load_env_var_from_file(env_path, ENV_VAR_NAME)
    if from_file:
        return Path(from_file)

    from_env = os.getenv(ENV_VAR_NAME)
    if from_env:
        return Path(from_env)

    raise RuntimeError(
        f"{ENV_VAR_NAME} not found. Provide in_path, set it in .env, or set it as an environment variable."
    )


def _find_model_root(mods_path: Path) -> Path:
    primary = mods_path / "SKSE" / "SexLab" / "ModelData"
    if primary.exists():
        return primary

    print(f"ModelData path not found at {primary}, checking overwrite directory...")
    overwrite = mods_path / ".." / "overwrite" / "SKSE" / "SexLab" / "ModelData"
    if overwrite.exists():
        return overwrite

    raise FileNotFoundError(f"ModelData path not found: {primary}")


def load_data(
    in_path: str | None = None,
    out_path: str | Path | None = None,
) -> dict[str, pd.DataFrame]:
    mods_path = _resolve_mods_path(in_path)
    model_root = _find_model_root(mods_path)

    dataframes: dict[str, list[pd.DataFrame]] = defaultdict(list)
    for cluster_dir in sorted(p for p in model_root.iterdir() if p.is_dir()):
        for csv_path in sorted(cluster_dir.glob("*.csv")):
            dataframes[cluster_dir.name].append(pd.read_csv(csv_path))

    if not dataframes:
        raise FileNotFoundError(f"No CSV files found under: {model_root}")

    output_root = Path(out_path) if out_path else None
    if output_root:
        output_root.mkdir(parents=True, exist_ok=True)

    merged: dict[str, pd.DataFrame] = {}
    for cluster_name, dfs in dataframes.items():
        df = pd.concat(dfs, ignore_index=True)
        merged[cluster_name] = df
        if output_root:
            df.to_csv(output_root / f"{cluster_name}_interactions.csv", index=False)

    return merged

if __name__ == "__main__":
    out_path = Path(__file__).resolve().parent / "out"
    load_data(None, out_path)

