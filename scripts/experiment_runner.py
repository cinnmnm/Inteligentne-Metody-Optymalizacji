from __future__ import annotations

import json
import random
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

import pandas as pd


def _get_vertex_count(instance_path: Path) -> int:
    count = 0
    with instance_path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.strip():
                count += 1
    return count


def build_execution_plan(
    instances: Dict[str, Path],
    start_nodes_per_instance: int,
    runs_per_instance: int,
    seed: int,
) -> List[tuple[str, Path, int, int]]:
    rng = random.Random(seed)
    plan: List[tuple[str, Path, int, int]] = []

    for instance_name, instance_path in instances.items():
        max_node = _get_vertex_count(instance_path) - 1
        if max_node < 0:
            raise RuntimeError(f"Instance has no vertices: {instance_path}")

        start_node_count = min(start_nodes_per_instance, max_node + 1)
        start_nodes = rng.sample(range(max_node + 1), k=start_node_count)

        for start_node in start_nodes:
            for _ in range(runs_per_instance):
                run_seed = rng.randint(0, 2**31 - 1)
                plan.append((instance_name, instance_path, start_node, run_seed))

    return plan


@dataclass
class ExperimentRunner:
    binary_path: Path
    instances: Dict[str, Path]
    solvers: List[str]
    runs_per_instance: int
    start_nodes_per_instance: int
    seed: int = 80081355
    solver_extra_args: Dict[str, List[str]] | None = None
    execution_plan: List[tuple[str, Path, int, int]] | None = None

    def run(self) -> pd.DataFrame:
        if self.execution_plan is None:
            execution_plan = build_execution_plan(
                instances=self.instances,
                start_nodes_per_instance=self.start_nodes_per_instance,
                runs_per_instance=self.runs_per_instance,
                seed=self.seed,
            )
        else:
            execution_plan = self.execution_plan

        rows: List[dict] = []
        for instance_name, instance_path, start_node, run_seed in execution_plan:
            for solver_name in self.solvers:
                payload = self._run_single(instance_path, solver_name, start_node, run_seed)
                payload["instance"] = instance_name
                payload["seed"] = run_seed
                rows.append(payload)

        return pd.DataFrame(rows)

    def _run_single(self, instance_path: Path, solver_name: str, start_node: int, seed: int) -> dict:
        print(f"Uruchamiam: instance={instance_path.name} solver={solver_name} start_node={start_node} seed={seed}")

        cmd = [
            str(self.binary_path),
            str(instance_path),
            solver_name,
            str(start_node),
            str(seed),
        ]

        if self.solver_extra_args and solver_name in self.solver_extra_args:
            cmd.extend(self.solver_extra_args[solver_name])

        completed = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
        )

        if completed.returncode != 0:
            raise RuntimeError(
                "Solver process failed with non-zero exit code. "
                f"command={' '.join(cmd)} returncode={completed.returncode} stderr={completed.stderr.strip()}"
            )

        stdout = completed.stdout.strip()
        if not stdout:
            raise RuntimeError(f"Solver returned empty stdout for command: {' '.join(cmd)}")

        try:
            return json.loads(stdout)
        except json.JSONDecodeError as ex:
            raise RuntimeError(
                f"Failed to parse solver JSON output. command={' '.join(cmd)} stdout={stdout}"
            ) from ex


def save_results(df: pd.DataFrame, results_dir: Path) -> tuple[Path, Path]:
    results_dir.mkdir(parents=True, exist_ok=True)

    csv_path = results_dir / "experiment_results.csv"
    json_path = results_dir / "experiment_results.json"

    df.to_csv(csv_path, index=False)
    df.to_json(json_path, orient="records", indent=4)

    return csv_path, json_path
