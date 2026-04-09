from __future__ import annotations

import argparse
import pandas as pd
import ast
from pathlib import Path
from solution_visualizer import SolutionVisualizer

def generate_best_solution_plots(df: pd.DataFrame, instances: dict[str, Path], results_dir: Path) -> None:
    print("\nGenerowanie wizualizacji...")
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

                best_path = best_row["path"]
                if isinstance(best_path, str):
                    best_path = ast.literal_eval(best_path)

                instance_file = instances[instance_name]
                save_file = results_dir / f"best_{instance_name}_{solver_name}.png"
                title = f"{instance_name} | {solver_name} | Obj: {best_row['final_objective']}"

                SolutionVisualizer.plot_solution(instance_file, best_path, save_file, title)
                print(f" -> Zapisano wykres: {save_file.name}")
            else:
                print(f" -> Brak poprawnych rozwiązań dla {instance_name} - {solver_name}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate visualizations from experiment results.")
    parser.add_argument("--lab", default="lab1", help="Lab name, e.g. lab1, lab2")
    parser.add_argument("--results-dir", default=None, help="Optional explicit path to results directory")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    results_dir = Path(args.results_dir) if args.results_dir else root / "results" / args.lab
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