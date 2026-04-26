#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

if [[ $# -lt 1 ]]; then
	echo "Usage: $0 <lab_name> [main.py args...]"
	exit 1
fi

LAB_NAME="$1"
shift

cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build"
cmake --build "$ROOT_DIR/build"

python "$ROOT_DIR/scripts/main.py" --lab "$LAB_NAME" "$@"