import pandas as pd
import ast
from pathlib import Path
from solution_visualizer import SolutionVisualizer

def main():
    root = Path(__file__).resolve().parent
    results_dir = root / "results"
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

    print("\nGenerowanie wizualizacji...")
    for instance_name in df['instance'].unique():
        for solver_name in df['solver'].unique():
            
            df_valid = df[(df['instance'] == instance_name) & 
                          (df['solver'] == solver_name) & 
                          (df['is_valid'] == True)]
            
            if not df_valid.empty:
                best_idx = df_valid['final_objective'].idxmax()
                best_row = df_valid.loc[best_idx]
                
                best_path = ast.literal_eval(best_row['path'])
                instance_file = instances[instance_name]
                
                save_file = results_dir / f"best_{instance_name}_{solver_name}.png"
                title = f"{instance_name} | {solver_name} | Obj: {best_row['final_objective']}"
                
                SolutionVisualizer.plot_solution(instance_file, best_path, save_file, title)
                print(f" -> Zapisano wykres: {save_file.name}")
            else:
                print(f" -> Brak poprawnych rozwiązań dla {instance_name} - {solver_name}")

if __name__ == "__main__":
    main()