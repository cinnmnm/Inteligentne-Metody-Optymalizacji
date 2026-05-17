from __future__ import annotations

import json
import subprocess
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed

import pandas as pd
import matplotlib.pyplot as plt

def run_single(cmd, inst_name, solver_name, k, start_node):
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
        data = json.loads(proc.stdout)
        
        return {
            "instance": inst_name,
            "solver": solver_name,
            "base_solver": solver_name.split("_k")[0],
            "k_value": k,
            "start_node": start_node,
            "final_objective": data["final_objective"],
            "time_ms": data["time_ms"]
        }
    except Exception as e:
        print(f"Error running {cmd}")
        return None

def main() -> None:
    root = Path(__file__).resolve().parent.parent
    binary_path = root / "build" / "lab3" / "solver_lab3"
    
    if not binary_path.exists():
        binary_path = root / "build" / "solver_lab3"
        if not binary_path.exists():
            raise RuntimeError(f"Binary not found. Looked for {binary_path}")

    instances = {
        "TSPA": root / "instances" / "TSPA.csv",
        "TSPB": root / "instances" / "TSPB.csv",
    }

    k_values = [5, 10, 15, 20, 25, 50, 100, 200]
    num_start_nodes = 100
    solver_names = ["candidate", "list_memory", "hybrid", "lazy_pq"]
    ignored_init = "heuristic"
    max_time_ms = 1000
    seed = 123456789

    results_dir = root / "results" / "lab3"
    results_dir.mkdir(parents=True, exist_ok=True)

    records = []
    tasks = []

    print("Running k-value experiments...")
    with ProcessPoolExecutor() as executor:
        for inst_name, inst_path in instances.items():
            for k in k_values:
                for solver_name in solver_names:
                    for start_node in range(num_start_nodes):
                        cmd = [
                            str(binary_path),
                            str(inst_path),
                            solver_name,
                            str(start_node),
                            str(seed),
                            ignored_init,
                            str(max_time_ms),
                            str(k),
                        ]
                        tasks.append(executor.submit(run_single, cmd, inst_name, f"{solver_name}_k{k}", k, start_node))
        
        for i, future in enumerate(as_completed(tasks)):
            if i % 100 == 0:
                print(f"Completed {i}/{len(tasks)} runs...")
            res = future.result()
            if res:
                records.append(res)

    df = pd.DataFrame(records)
    if df.empty:
        print("No results collected.")
        return

    summary_df = df.groupby(["instance", "solver", "base_solver", "k_value"]).agg(
        min_final_objective=("final_objective", "min"),
        max_final_objective=("final_objective", "max"),
        mean_final_objective=("final_objective", "mean"),
        mean_time_ms=("time_ms", "mean")
    ).reset_index()

    summary_csv = results_dir / "candidate_k_summary.csv"
    summary_df.drop(columns=["k_value"]).to_csv(summary_csv, index=False)
    print(f"\nSaved summary to {summary_csv}")
    print(summary_df.drop(columns=["k_value"]))

    for inst_name in instances.keys():
        inst_df = summary_df[summary_df["instance"] == inst_name]
        
        if inst_df.empty:
            continue
            
        fig, axes = plt.subplots(1, 2, figsize=(12, 5))
        fig.suptitle(f"{inst_name}: Lab 3 Algorithms Performance vs k")
        
        colors = ['blue', 'green', 'red', 'purple']
        for idx, base_s in enumerate(solver_names):
            s_df = inst_df[inst_df["base_solver"] == base_s].sort_values("k_value")
            
            axes[0].plot(s_df["k_value"], s_df["mean_time_ms"], marker='o', linestyle='-', color=colors[idx % len(colors)], label=base_s)
            axes[1].plot(s_df["k_value"], s_df["mean_final_objective"], marker='o', linestyle='-', color=colors[idx % len(colors)], label=base_s)
            
        axes[0].set_title("Mean Execution Time vs k")
        axes[0].set_xlabel("k (Nearest Neighbors)")
        axes[0].set_ylabel("Mean Time (ms)")
        axes[0].grid(True)
        axes[0].legend()
        
        axes[1].set_title("Mean Final Objective vs k")
        axes[1].set_xlabel("k (Nearest Neighbors)")
        axes[1].set_ylabel("Mean Final Objective")
        axes[1].grid(True)
        axes[1].legend()
        
        plot_path = results_dir / f"candidate_k_plot_{inst_name}.png"
        plt.tight_layout()
        plt.savefig(plot_path)
        plt.close()
        print(f"Saved plot to {plot_path}")

if __name__ == "__main__":
    main()
