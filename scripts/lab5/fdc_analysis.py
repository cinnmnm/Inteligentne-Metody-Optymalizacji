import json
import random
import subprocess
from pathlib import Path
from typing import List, Dict, Set, Tuple
import itertools
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import pearsonr

# Configuration 
RESULTS_DIR = Path("/workspaces/Inteligentne-Metody-Optymalizacji/results")
INSTANCES_DIR = Path("/workspaces/Inteligentne-Metody-Optymalizacji/instances")
LAB5_RESULTS_DIR = RESULTS_DIR / "lab5"
BINARY_PATH = Path("/workspaces/Inteligentne-Metody-Optymalizacji/build/lab2/solver_lab2")

# Using greedy node/edge from lab2, adjust if necessary
SOLVER_NAME = "greedy_edge" #candidate
RUNS_PER_INSTANCE = 1000

def get_vertices(path: List[int]) -> Set[int]:
    """Returns the set of unique vertices in the path (excluding the redundant return link)."""
    return set(path[:-1]) if len(path) > 0 else set()

def get_edges(path: List[int]) -> Set[Tuple[int, int]]:
    """Returns the set of undirected edges in the path."""
    edges = set()
    for i in range(len(path) - 1):
        u, v = path[i], path[i+1]
        edges.add((min(u, v), max(u, v)))
    return edges

def find_global_best() -> Dict[str, dict]:
    """Finds the global best solution (maximum final_objective) for each instance across all previous labs."""
    best_solutions = {}
    
    for json_file in RESULTS_DIR.glob("lab*/experiment_results.json"):
        if "lab5" in json_file.parts:
             continue
             
        try:
            with open(json_file, 'r', encoding='utf-8') as f:
                data = json.load(f)
                
            for entry in data:
                instance = entry.get("instance")
                objective = entry.get("final_objective")
                path = entry.get("path")
                
                if instance and objective is not None and path:
                    if instance not in best_solutions or objective > best_solutions[instance]["final_objective"]:
                        best_solutions[instance] = {
                            "final_objective": objective,
                            "path": path,
                            "source_file": str(json_file),
                            "solver": entry.get("solver")
                        }
        except Exception as e:
            print(f"Warning: Could not read {json_file}: {e}")
            
    print("--- GLOBAL BEST SOLUTIONS ---")
    for inst, b in best_solutions.items():
        print(f"[{inst}] Obj: {b['final_objective']} (Solver: {b['solver']} from {b['source_file']})")
    
    return best_solutions

def run_local_optima(instance_name: str, instance_path: Path) -> List[dict]:
    """Runs the solver 1000 times with random seeds."""
    print(f"Running {RUNS_PER_INSTANCE} executions for {instance_name}...")
    results = []
    
    # Fast path for existing lab5 results:
    cache_file = LAB5_RESULTS_DIR / f"{instance_name}_1000_runs.json"
    if cache_file.exists():
        print(f"Loading cached runs from {cache_file}...")
        with open(cache_file, 'r') as f:
            return json.load(f)

    # Note: to accurately run, we will just pick a random start_node each time (e.g. 0 to 99).
    for i in range(RUNS_PER_INSTANCE):
        seed = random.randint(0, 2**31 - 1)
        start_node = random.randint(0, 50) # Fallback roughly
        
        cmd = [str(BINARY_PATH), str(instance_path), SOLVER_NAME, str(start_node), str(seed)]
        try:
            res = subprocess.run(cmd, capture_output=True, text=True, check=False)
            if res.returncode == 0 and res.stdout.strip():
                try:
                    payload = json.loads(res.stdout.strip())
                    results.append(payload)
                except json.JSONDecodeError:
                    pass
        except Exception as e:
            print(f"Error running solver: {e}")
            
    with open(cache_file, 'w') as f:
        json.dump(results, f)
        
    return results

def compute_similarity(population: List[dict], best_path: List[int]):
    """Computes similarity metrics for the population."""
    best_v = get_vertices(best_path)
    best_e = get_edges(best_path)
    
    pop_v = [get_vertices(sol["path"]) for sol in population]
    pop_e = [get_edges(sol["path"]) for sol in population]
    n = len(population)
    
    sim_best_v = [len(pv.intersection(best_v)) for pv in pop_v]
    sim_best_e = [len(pe.intersection(best_e)) for pe in pop_e]
    
    # Average pairwise similarity
    sum_sim_v = np.zeros(n)
    sum_sim_e = np.zeros(n)
    
    print("Computing average pairwise similarities...")
    for i, j in itertools.combinations(range(n), 2):
        intersect_v = len(pop_v[i].intersection(pop_v[j]))
        sum_sim_v[i] += intersect_v
        sum_sim_v[j] += intersect_v
        
        intersect_e = len(pop_e[i].intersection(pop_e[j]))
        sum_sim_e[i] += intersect_e
        sum_sim_e[j] += intersect_e
        
    sim_avg_v = sum_sim_v / (n - 1) if n > 1 else np.zeros(n)
    sim_avg_e = sum_sim_e / (n - 1) if n > 1 else np.zeros(n)
    
    return sim_best_v, sim_avg_v, sim_best_e, sim_avg_e

def plot_all_instances(instances_data: Dict[str, dict]):
    """
    Oczekiwany format wejściowy 'instances_data':
    {
        "TSPA": {
            "objectives": [...], # Lista 1000 wyników
            "metrics": (sim_best_v, sim_avg_v, sim_best_e, sim_avg_e) # 4 listy po 1000 wyników
        },
        "TSPB": {
            "objectives": [...],
            "metrics": (sim_best_v, sim_avg_v, sim_best_e, sim_avg_e)
        }
    }
    """
    # Proporcje strony A4 to ok. 1:1.41. Rozmiar 10x14 cali będzie idealny
    fig, axs = plt.subplots(4, 2, figsize=(10, 14.14))
    fig.suptitle("Fitness-Distance Correlation (FDC)", fontsize=18, fontweight='bold', y=0.98)
    
    out_records = []
    
    # Konfiguracja wierszy (4 metryki)
    metric_configs = [
        ("Podobieństwo do najlepszego (Wierzchołki)", "Best_V"),
        ("Średnie podobieństwo do populacji (Wierzchołki)", "Avg_V"),
        ("Podobieństwo do najlepszego (Krawędzie)", "Best_E"),
        ("Średnie podobieństwo do populacji (Krawędzie)", "Avg_E")
    ]
    
    # Iterujemy po kolumnach (instancjach)
    for col_idx, (instance_name, data) in enumerate(instances_data.items()):
        objectives = data["objectives"]
        metrics = data["metrics"]
        
        # Iterujemy po wierszach (metrykach)
        for row_idx, (title_base, metric_id) in enumerate(metric_configs):
            ax = axs[row_idx, col_idx]
            y_vals = metrics[row_idx]
            
            # Liczymy korelację DLA 1000 PUNKTÓW
            r, _ = pearsonr(objectives, y_vals)
            
            # 1. Rysujemy kropki (1000 losowych optimów, BEZ Global Best)
            ax.scatter(objectives, y_vals, alpha=0.4, edgecolor='k', s=20, label='Optima lokalne')
            
            # 2. LICZYMY LINIĘ TRENDU (Regresja liniowa: stopień wielomianu = 1)
            # polyfit zwraca współczynniki [a, b] dla prostej y = ax + b
            a, b = np.polyfit(objectives, y_vals, 1)
            
            # Tworzymy punkty X dla linii (od najmniejszego do największego objective w zbiorze)
            x_trend = np.array([min(objectives), max(objectives)])
            y_trend = a * x_trend + b
            
            # 3. RYSUJEMY LINIĘ TRENDU
            ax.plot(x_trend, y_trend, color='red', linestyle='--', linewidth=2, label='Linia trendu')
            
            # Liczymy korelację
            r, _ = pearsonr(objectives, y_vals)
            
            # Tytuł i konfiguracja
            ax.set_title(f"{instance_name} | {title_base}\nr = {r:.3f}", fontsize=11)
            ax.set_xlabel("Wartość funkcji celu")
            ax.set_ylabel("Podobieństwo")
            ax.grid(True, linestyle='--', alpha=0.6)
            ax.legend(fontsize=9)  # Warto dodać małą legendę, żeby opis brzmiał profesjonalnie
            
            out_records.append({
                "Instance": instance_name,
                "Metric_Type": metric_id,
                "Correlation": r
            })
            
    # Dopasowanie marginesów, żeby tytuł główny (suptitle) nie nachodził na wykresy
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    
    # Zapisz jako jeden plik o wysokiej rozdzielczości
    plot_file = LAB5_RESULTS_DIR / "fdc_plots_combined_A4.png"
    plt.savefig(plot_file, dpi=300)
    plt.close()
    
    return out_records

def main():
    LAB5_RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    
    global_bests = find_global_best()
    if not global_bests:
        print("No global bests found. Ensure previous lab results exist.")
        return

    instances_data = {}
    
    # Process each instance
    for instance_path in INSTANCES_DIR.glob("*.csv"):
        instance_name = instance_path.stem
        
        if instance_name not in global_bests:
            print(f"Skipping {instance_name}, no global best found.")
            continue
            
        best_sol = global_bests[instance_name]
        
        # Run local optimas
        population_results = run_local_optima(instance_name, instance_path)
        
        if len(population_results) < 2:
            print(f"Not enough results generated for {instance_name}")
            continue
            
        objectives = [r["final_objective"] for r in population_results]
        
        # Compute metrics
        metrics = compute_similarity(population_results, best_sol["path"])
        
        instances_data[instance_name] = {
            "objectives": objectives,
            "metrics": metrics
        }
        
    all_correlations = plot_all_instances(instances_data)

    # Save summary 
    if all_correlations:
        df = pd.DataFrame(all_correlations)
        csv_path = LAB5_RESULTS_DIR / "fdc_correlations.csv"
        df.to_csv(csv_path, index=False)
        print(f"Correlations saved to {csv_path}")
        print(df)

if __name__ == "__main__":
    main()
