from __future__ import annotations

import argparse
import pandas as pd
import ast
from pathlib import Path
from solution_visualizer import SolutionVisualizer


def _parse_path(value):
    if isinstance(value, list):
        return value
    if isinstance(value, str):
        text = value.strip()
        if not text:
            return None
        return ast.literal_eval(text)
    return None


def resolve_results_dir(root: Path, lab_name: str, results_dir: str | None, initial_solution_type: str) -> Path:
    if results_dir:
        return Path(results_dir)

    if lab_name == "lab1":
        return root / "results" / lab_name
    elif lab_name == "lab2":
        return root / "results" / lab_name / initial_solution_type
    elif lab_name == "lab3":
        return root / "results" / lab_name
    else:
        raise ValueError(f"Unsupported lab name: {lab_name}. Expected lab1, lab2, lab3, etc.")


def generate_best_solution_plots(df: pd.DataFrame, instances: dict[str, Path], results_dir: Path) -> None:
    print("\nGenerowanie wizualizacji...")
    has_initial_path = "initial_path" in df.columns

    for instance_name in df["instance"].unique():
        for solver_name in df["solver"].unique():
            df_valid = df[
                (df["instance"] == instance_name)
                & (df["solver"] == solver_name)
                & (df["is_valid"] == True)
            ]

            if not df_valid.empty:
                best_idx = df_valid["final_objective"].idxmax()
                best_row = df_valid.loc[best_idx]

                best_path = _parse_path(best_row["path"])
                if not best_path:
                    print(f" -> Brak ścieżki końcowej dla {instance_name} - {solver_name}")
                    continue

                instance_file = instances[instance_name]
                save_file = results_dir / f"best_{instance_name}_{solver_name}.png"
                title = f"{instance_name} | {solver_name} | Obj: {best_row['final_objective']}"

                SolutionVisualizer.plot_solution(instance_file, best_path, save_file, title)
                print(f" -> Zapisano wykres: {save_file.name}")

                if has_initial_path:
                    initial_path = _parse_path(best_row.get("initial_path"))
                    if initial_path:
                        initial_file = results_dir / f"start_{instance_name}_{solver_name}.png"
                        initial_objective = best_row.get("initial_objective", best_row.get("phase1_objective"))
                        initial_title = (
                            f"{instance_name} | {solver_name} | "
                            f"Start Obj: {initial_objective}"
                        )
                        SolutionVisualizer.plot_solution(instance_file, initial_path, initial_file, initial_title)
                        print(f" -> Zapisano wykres: {initial_file.name}")
            else:
                print(f" -> Brak poprawnych rozwiązań dla {instance_name} - {solver_name}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate visualizations from experiment results.")
    parser.add_argument("--lab", default="lab1", help="Lab name, e.g. lab1, lab2, lab3")
    parser.add_argument("--results-dir", default=None, help="Optional explicit path to results directory")
    parser.add_argument(
        "--initial-solution-type",
        default="heuristic",
        choices=["random", "heuristic"],
        help="Only used when resolving default lab2 results path",
    )
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    results_dir = resolve_results_dir(root, args.lab, args.results_dir, args.initial_solution_type)
    csv_path = results_dir / "experiment_results.csv"

    if not csv_path.exists():
        print(f"Brak pliku wyników: {csv_path}")
        return

    instances = {
        "TSPA": root / "instances" / "TSPA.csv",
        "TSPB": root / "instances" / "TSPB.csv",
    }

    print(f"Wczytywanie wyników z {csv_path}...")
    df = pd.read_csv(csv_path)

    generate_best_solution_plots(df, instances, results_dir)

if __name__ == "__main__":
    main()