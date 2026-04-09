from __future__ import annotations

import argparse
from pathlib import Path

from experiment_runner import ExperimentRunner, save_results
from plot_results import generate_best_solution_plots


def main() -> None:
    parser = argparse.ArgumentParser(description="Run experiments for selected lab and save outputs.")
    parser.add_argument("--lab", default="lab1", help="Lab name, e.g. lab1, lab2")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    binary_name = f"solver_{args.lab}"
    binary_candidates = [
        root / "build" / args.lab / binary_name,
        root / "build" / binary_name,
    ]
    binary_path = next((path for path in binary_candidates if path.exists()), binary_candidates[0])

    if not binary_path.exists():
        raise RuntimeError(
            "Binary not found for selected lab. "
            f"Expected one of: {', '.join(str(path) for path in binary_candidates)}. "
            "Build C++ first (cmake -S . -B build; cmake --build build)."
        )

    instances = {
        "TSPA": root / "instances" / "TSPA.csv",
        "TSPB": root / "instances" / "TSPB.csv",
    }

    runner = ExperimentRunner(
        binary_path=binary_path,
        instances=instances,
        solvers=[
            "random", 
            "nearest_neighbor", 
            "nearest_neighbor_profit", 
            "greedy_cycle", 
            "greedy_cycle_profit", 
            "regret", 
            "weighted_regret"
        ],
        runs_per_instance=1,
        start_nodes_per_instance=200, 
        seed=8008136745555,
    )

    df = runner.run()

    results_dir = root / "results" / args.lab
    csv_path, json_path = save_results(df, results_dir)

    print(f"\nGotowe! Wyniki zapisano w folderze: {results_dir}")
    print(f"CSV: {csv_path.name}")
    print(f"JSON: {json_path.name}")

    print("\nGenerowanie wizualizacji...")
    generate_best_solution_plots(df, instances, results_dir)

if __name__ == "__main__":
    main()