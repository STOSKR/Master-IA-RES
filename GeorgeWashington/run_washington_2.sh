#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington-train
#SBATCH --cpus-per-task=8
#SBATCH --mem=16G
#SBATCH --gres=shard:1
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh
conda activate res_env

mkdir -p logs models/Optical

export PYTORCH_ALLOC_CONF="${PYTORCH_ALLOC_CONF:-expandable_segments:True}"

if [ -n "${CUDA_VISIBLE_DEVICES:-}" ]; then
  echo "Using GPU(s): ${CUDA_VISIBLE_DEVICES}"
fi

if command -v nvidia-smi >/dev/null 2>&1; then
  nvidia-smi --query-gpu=index,name,memory.total,memory.free --format=csv || true
fi

TRAIN_DIR="${TRAIN_DIR:-data/data_train_rodrigo_vocab_clean}"
VAL_DIR="${VAL_DIR:-data/data_train_rodrigo_vocab_clean}"
OUT_MODEL="${OUT_MODEL:-models/Optical/washington_2.pth}"

for path in "${TRAIN_DIR}" "${VAL_DIR}"; do
  if [ ! -d "${path}" ]; then
    echo "ERROR: required dataset directory not found: ${path}" >&2
    exit 1
  fi
done

echo "=========================================="
echo "Starting Washington HTR training: $(date)"
echo "Project: ${PROJECT_DIR}"
echo "Train dir: ${TRAIN_DIR}"
echo "Val dir: ${VAL_DIR}"
echo "Output model: ${OUT_MODEL}"
echo "=========================================="

python HTRnn/train.py \
  --data-augm \
  --epochs 2000 \
  --early-stop 15 \
  --space-symbol "~" \
  --batch-size 1 \
  --lr 1e-4 \
  --finetune-lr 5e-5 \
  "${TRAIN_DIR}" \
  "${VAL_DIR}" \
  "${OUT_MODEL}"

echo "=========================================="
echo "Finished Washington HTR training: $(date)"
ls -lh "${OUT_MODEL}" || true
echo "=========================================="
