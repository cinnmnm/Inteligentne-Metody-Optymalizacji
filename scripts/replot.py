import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

def main():
    root = Path(__file__).resolve().parent.parent
    results_dir = root / "results" / "lab3"
    summary_csv = results_dir / "candidate_k_summary.csv"

    if not summary_csv.exists():
        print(f"Error: Could not find {summary_csv}")
        return

    summary_df = pd.read_csv(summary_csv)
    instances = summary_df["instance"].unique()
    
    target_solvers = ["candidate", "hybrid"]
    colors = ['blue', 'green']

    for inst_name in instances:
        inst_df = summary_df[summary_df["instance"] == inst_name]
        
        if inst_df.empty:
            continue
            
        fig, axes = plt.subplots(1, 2, figsize=(12, 5))
        fig.suptitle(f"{inst_name}: Candidate vs Hybrid Performance vs k")
        
        lm_df = inst_df[inst_df["solver"] == "list_memory"]
        if lm_df.empty:
             lm_df = inst_df[inst_df["base_solver"] == "list_memory"]
             
        lm_time = None
        lm_obj = None
        
        if not lm_df.empty:
            lm_time = lm_df["mean_time_ms"].mean()
            lm_obj = lm_df["mean_final_objective"].mean()
        
        for idx, base_s in enumerate(target_solvers):
            s_df = inst_df[inst_df["base_solver"] == base_s].copy()
            if s_df.empty:
                continue
                
            s_df["k_value"] = s_df["solver"].str.extract(r'(\d+)').astype(int)
            s_df = s_df.sort_values("k_value")
            
            axes[0].plot(s_df["k_value"], s_df["mean_time_ms"], marker='o', linestyle='-', color=colors[idx % len(colors)], label=base_s)
            axes[1].plot(s_df["k_value"], s_df["mean_final_objective"], marker='o', linestyle='-', color=colors[idx % len(colors)], label=base_s)
            
        if lm_time is not None:
            axes[0].axhline(y=lm_time, color='red', linestyle='--', linewidth=1.5, label='list_memory')
        if lm_obj is not None:
            axes[1].axhline(y=lm_obj, color='red', linestyle='--', linewidth=1.5, label='list_memory')

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