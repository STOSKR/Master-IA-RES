#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington-paso1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

# Create a deterministic 80/10/10 split from the current Washington training set.

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

mkdir -p logs

SOURCE_DIR="${SOURCE_DIR:-data/data_train_rodrigo_vocab_clean}"
TRAIN_OUT="${TRAIN_OUT:-data/data_train_80}"
VAL_OUT="${VAL_OUT:-data/data_val_10}"
TEST_OUT="${TEST_OUT:-data/data_test_10}"
LIST_DIR="${LIST_DIR:-data/lists}"
SEED="${SEED:-2026}"

TRAIN_LIST="${TRAIN_LIST:-${LIST_DIR}/train_80.lst}"
VAL_LIST="${VAL_LIST:-${LIST_DIR}/val_10.lst}"
TEST_LIST="${TEST_LIST:-${LIST_DIR}/test_10.lst}"

if [ ! -d "${SOURCE_DIR}" ]; then
  echo "ERROR: source directory not found: ${SOURCE_DIR}" >&2
  exit 1
fi

mkdir -p "${LIST_DIR}"

for out_dir in "${TRAIN_OUT}" "${VAL_OUT}" "${TEST_OUT}"; do
  if [ -e "${out_dir}" ]; then
    echo "Removing previous split directory: ${out_dir}"
    rm -rf "${out_dir}"
  fi
  mkdir -p "${out_dir}"
done

python - <<'PY' "${SOURCE_DIR}" "${TRAIN_LIST}" "${VAL_LIST}" "${TEST_LIST}" "${SEED}"
import random
import sys
from pathlib import Path

source_dir = Path(sys.argv[1])
train_list = Path(sys.argv[2])
val_list = Path(sys.argv[3])
test_list = Path(sys.argv[4])
seed = int(sys.argv[5])

items = []
for txt_path in sorted(source_dir.glob("*.txt")):
    stem = txt_path.stem
    jpg_path = source_dir / f"{stem}.jpg"
    if jpg_path.exists():
        items.append(stem)
    else:
        print(f"WARNING: missing image for {stem}; skipping", file=sys.stderr)

random.Random(seed).shuffle(items)

n = len(items)
n_train = int(n * 0.8)
remaining = n - n_train
n_val = remaining // 2

splits = {
    train_list: items[:n_train],
    val_list: items[n_train:n_train + n_val],
    test_list: items[n_train + n_val:],
}

for path, split_items in splits.items():
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(split_items) + ("\n" if split_items else ""), encoding="utf-8")

print(f"Total items: {n}")
print(f"Train: {len(splits[train_list])} -> {train_list}")
print(f"Val:   {len(splits[val_list])} -> {val_list}")
print(f"Test:  {len(splits[test_list])} -> {test_list}")
PY

link_split() {
  local list_file="$1"
  local out_dir="$2"

  while IFS= read -r item; do
    [ -n "${item}" ] || continue
    ln -s "../../${SOURCE_DIR}/${item}.jpg" "${out_dir}/${item}.jpg"
    ln -s "../../${SOURCE_DIR}/${item}.txt" "${out_dir}/${item}.txt"
  done < "${list_file}"
}

link_split "${TRAIN_LIST}" "${TRAIN_OUT}"
link_split "${VAL_LIST}" "${VAL_OUT}"
link_split "${TEST_LIST}" "${TEST_OUT}"

echo "=========================================="
echo "Split created successfully"
echo "Source: ${SOURCE_DIR}"
echo "Seed: ${SEED}"
echo "Train dir: ${TRAIN_OUT} ($(wc -l < "${TRAIN_LIST}") items)"
echo "Val dir:   ${VAL_OUT} ($(wc -l < "${VAL_LIST}") items)"
echo "Test dir:  ${TEST_OUT} ($(wc -l < "${TEST_LIST}") items)"
echo "=========================================="
