#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=latgen-chars
#SBATCH --cpus-per-task=1
#SBATCH --mem=32G
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/Rodrigo
#SBATCH -o logs/%x-%j.log
# No --gres=shard here: latgen-faster-mapped is CPU-only.

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/Rodrigo}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh
conda activate kaldi_env

mkdir -p logs results/lattices8

BEAM="${BEAM:-25.0}"
MAX_ACTIVE="${MAX_ACTIVE:-5000}"
ACOUSTIC_SCALE="${ACOUSTIC_SCALE:-1.0}"

MODEL="${MODEL:-models/WFST/model.hmm}"
GRAPH="${GRAPH:-models/WFST/TG.fst}"
FEATS="${FEATS:-models/Optical/Conf_mat_test.ark}"
SYMBOLS="${SYMBOLS:-models/WFST/tokensMap.txt}"

LAT_OUT="${LAT_OUT:-results/lattices8/test-char-lat.raw.gz}"
HYP_OUT="${HYP_OUT:-results/test-8gram-chars-1bestLat.hyp}"

JOB_TAG="${SLURM_JOB_ID:-$$}"
LAT_TMP="${LAT_OUT}.partial.${JOB_TAG}"
HYP_TMP="${HYP_OUT}.partial.${JOB_TAG}"

echo "=========================================="
echo "Starting latgen-faster-mapped: $(date)"
echo "Project: ${PROJECT_DIR}"
echo "Job id: ${JOB_TAG}"
echo "Graph: ${GRAPH}"
echo "Features: ${FEATS}"
echo "Lattice output: ${LAT_OUT}"
echo "Hypothesis output: ${HYP_OUT}"
echo "=========================================="

for path in "${MODEL}" "${GRAPH}" "${FEATS}" "${SYMBOLS}"; do
  if [ ! -s "${path}" ]; then
    echo "ERROR: required input not found or empty: ${path}" >&2
    exit 1
  fi
done

rm -f "${LAT_TMP}" "${HYP_TMP}"

latgen-faster-mapped \
  --beam="${BEAM}" \
  --max-active="${MAX_ACTIVE}" \
  --acoustic-scale="${ACOUSTIC_SCALE}" \
  --allow-partial=true \
  --determinize-lattice=false \
  --word-symbol-table="${SYMBOLS}" \
  "${MODEL}" \
  "${GRAPH}" \
  "ark:${FEATS}" \
  "ark,t:|gzip -c > ${LAT_TMP}" \
  "ark,t:${HYP_TMP}"

mv -f "${LAT_TMP}" "${LAT_OUT}"
mv -f "${HYP_TMP}" "${HYP_OUT}"

echo "=========================================="
echo "Finished successfully: $(date)"
ls -lh "${LAT_OUT}" "${HYP_OUT}"
echo "Hypothesis lines: $(wc -l < "${HYP_OUT}")"
echo "=========================================="
