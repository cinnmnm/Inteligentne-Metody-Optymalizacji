from __future__ import annotations

import argparse
from pathlib import Path

import pandas as pd

from experiment_runner import ExperimentRunner, build_execution_plan, save_results
from plot_results import generate_best_solution_plots


def main() -> None:
    parser = argparse.ArgumentParser(description="Run experiments for selected lab and save outputs.")
    parser.add_argument("--lab", default="lab1", help="Lab name, e.g. lab1, lab2")
    parser.add_argument("--runs-per-instance", type=int, default=1, help="Number of runs per (instance, solver, start node)")
    parser.add_argument("--start-nodes-per-instance", type=int, default=200, help="Number of random start nodes per instance")
    parser.add_argument(
        "--initial-solution-type",
        default="heuristic",
        choices=["random", "heuristic"],
        help="Initial solution type for lab2 solvers",
    )
    parser.add_argument(
        "--random-walk-time-ms",
        type=int,
        default=None,
        help="Optional fixed random walk budget in ms; if omitted for lab2, uses slowest LS average time",
    )
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

    if args.lab == "lab2":
        execution_plan = build_execution_plan(
            instances=instances,
            start_nodes_per_instance=args.start_nodes_per_instance,
            runs_per_instance=args.runs_per_instance,
            seed=8008136745555,
        )

        ls_solvers = [
            "steepest_node",
            "steepest_edge",
            "greedy_node",
            "greedy_edge",
        ]

        runner_ls = ExperimentRunner(
            binary_path=binary_path,
            instances=instances,
            solvers=ls_solvers,
            runs_per_instance=args.runs_per_instance,
            start_nodes_per_instance=args.start_nodes_per_instance,
            seed=8008136745555,
            solver_extra_args={solver: [args.initial_solution_type] for solver in ls_solvers},
            execution_plan=execution_plan,
        )

        df_ls = runner_ls.run()
        mean_times_ms = df_ls.groupby("solver")["time_ms"].mean()
        slowest_avg_time_ms = max(1, int(round(float(mean_times_ms.max()))))
        random_walk_time_ms = args.random_walk_time_ms if args.random_walk_time_ms is not None else slowest_avg_time_ms

        print("\nLab2 timing calibration (avg time_ms per LS solver):")
        for solver_name, avg_time in mean_times_ms.items():
            print(f" - {solver_name}: {avg_time:.3f} ms")
        print(f"Random Walk budget set to: {random_walk_time_ms} ms")

        runner_rw = ExperimentRunner(
            binary_path=binary_path,
            instances=instances,
            solvers=["random_walk"],
            runs_per_instance=args.runs_per_instance,
            start_nodes_per_instance=args.start_nodes_per_instance,
            seed=8008136745555,
            solver_extra_args={"random_walk": [args.initial_solution_type, str(random_walk_time_ms)]},
            execution_plan=execution_plan,
        )

        df_rw = runner_rw.run()
        df = df_ls if df_rw.empty else pd.concat([df_ls, df_rw], ignore_index=True)
    else:
        solvers = [
            "random",
            "nearest_neighbor",
            "nearest_neighbor_profit",
            "greedy_cycle",
            "greedy_cycle_profit",
            "regret",
            "weighted_regret",
        ]
        runner = ExperimentRunner(
            binary_path=binary_path,
            instances=instances,
            solvers=solvers,
            runs_per_instance=args.runs_per_instance,
            start_nodes_per_instance=args.start_nodes_per_instance,
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