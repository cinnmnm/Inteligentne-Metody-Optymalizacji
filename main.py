from __future__ import annotations

from pathlib import Path

from experiment_runner import ExperimentRunner
from solution_visualizer import SolutionVisualizer


def main() -> None:
    root = Path(__file__).resolve().parent
    binary_path = root / "build" / "solver"

    if not binary_path.exists():
        raise RuntimeError(
            f"Binary not found: {binary_path}. Build C++ first (cmake -S . -B build; cmake --build build --config Release)."
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
    
    results_dir = root / "results"
    results_dir.mkdir(exist_ok=True)
    
    csv_path = results_dir / "experiment_results.csv"
    df.to_csv(csv_path, index=False)
    
    json_path = results_dir / "experiment_results.json"
    df.to_json(json_path, orient="records", indent=4)
    
    print(f"\nGotowe! Wyniki zapisano w folderze: {results_dir}")

    print("\nGenerowanie wizualizacji...")
    
    for instance_name in df['instance'].unique():
        for solver_name in df['solver'].unique():
            df_valid = df[(df['instance'] == instance_name) & 
                          (df['solver'] == solver_name) & 
                          (df['is_valid'] == True)]
            
            if not df_valid.empty:
                best_idx = df_valid['final_objective'].idxmax()
                best_row = df_valid.loc[best_idx]
                
                best_path = best_row['path']
                instance_file = instances[instance_name]
                
                save_file = results_dir / f"best_{instance_name}_{solver_name}.png"
                title = f"{instance_name} | {solver_name} | Obj: {best_row['final_objective']}"
                
                SolutionVisualizer.plot_solution(instance_file, best_path, save_file, title)
                print(f" -> Zapisano wykres: {save_file.name}")

if __name__ == "__main__":
    main()