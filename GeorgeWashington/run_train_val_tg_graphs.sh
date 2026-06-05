#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington-tg-graphs
#SBATCH --cpus-per-task=10
#SBATCH --mem=32G
#SBATCH --gres=shard:1
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh

mkdir -p data/data_train_val results/lattices8/chars models/Optical logs data/text

MODEL="${MODEL:-models/Optical/washington_26.pth}"
TRAIN_DIR="${TRAIN_DIR:-data/data_train_80}"
VAL_DIR="${VAL_DIR:-data/data_val_10}"
TRAIN_VAL_DIR="${TRAIN_VAL_DIR:-data/data_train_val}"
BATCH_SIZE="${BATCH_SIZE:-24}"
GPU_ID="${GPU_ID:-}"
HTR_GPU="${HTR_GPU:-0}"

CONF_MAT="${CONF_MAT:-models/Optical/Conf_mat_train_val.ark}"
WFST_HMM="${WFST_HMM:-models/WFST/model.hmm}"
GRAPH="${GRAPH:-models/WFST/TG.fst}"
SYMBOLS="${SYMBOLS:-models/WFST/tokensMap.txt}"

LAT_GZ="${LAT_GZ:-results/lattices8/train_val-char-lat.gz}"
HYP_1BEST="${HYP_1BEST:-results/lattices8/train_val-TG-chars.hyp}"
LAT_TXT="${LAT_TXT:-results/lattices8/train_val-char-lat}"
LAT_SLF_IN="${LAT_SLF_IN:-results/lattices8/train_val-char-lat.slf-input}"
SLF_OUT="${SLF_OUT:-results/lattices8/chars}"
REF_OUT="${REF_OUT:-data/text/train_val_words.ref}"
HYP_SYM="${HYP_SYM:-results/lattices8/train_val-TG-chars.sym.hyp}"
HYP2REF="${HYP2REF:-results/lattices8/train_val_TG_hyp2ref}"

echo "=========================================="
echo "Washington train+val character graphs"
echo "Project: ${PROJECT_DIR}"
echo "Model: ${MODEL}"
echo "Train dir: ${TRAIN_DIR}"
echo "Val dir: ${VAL_DIR}"
echo "Train+val dir: ${TRAIN_VAL_DIR}"
echo "Graph: ${GRAPH}"
echo "Confidence matrix: ${CONF_MAT}"
echo "GPU_ID: ${GPU_ID:-SLURM/default}"
echo "HTR GPU index: ${HTR_GPU}"
echo "=========================================="

for path in "${MODEL}" "${TRAIN_DIR}" "${VAL_DIR}" "${WFST_HMM}" "${GRAPH}" "${SYMBOLS}"; do
  if [ ! -e "${path}" ]; then
    echo "ERROR: required input not found: ${path}" >&2
    exit 1
  fi
done

echo "=========================================="
echo "1. Building train+val dataset"
echo "=========================================="
rm -rf "${TRAIN_VAL_DIR}"
mkdir -p "${TRAIN_VAL_DIR}"
cp -r "${TRAIN_DIR}"/* "${TRAIN_VAL_DIR}/"
cp -r "${VAL_DIR}"/* "${TRAIN_VAL_DIR}/"
echo "Train+val text files: $(find -L "${TRAIN_VAL_DIR}" -maxdepth 1 -name '*.txt' | wc -l)"

echo "=========================================="
echo "2. Extracting confidence matrices"
echo "=========================================="
conda activate res_env
if [ -n "${GPU_ID}" ]; then
  export CUDA_VISIBLE_DEVICES="${GPU_ID}"
fi
if command -v nvidia-smi >/dev/null 2>&1; then
  nvidia-smi --query-gpu=index,name,memory.total,memory.used,memory.free --format=csv || true
fi
python HTRnn/get_confMats_binary.py \
  --gpu "${HTR_GPU}" \
  --batch_size "${BATCH_SIZE}" \
  --conf_matrix "${CONF_MAT}" \
  "${MODEL}" \
  "${TRAIN_VAL_DIR}"

echo "=========================================="
echo "3. Getting character graphs with Kaldi"
echo "=========================================="
conda activate kaldi_env

LATGEN="latgen-faster-mapped-parallel"
LATGEN_THREADS_ARGS=(--num-threads=10)
if ! command -v "${LATGEN}" >/dev/null 2>&1; then
  LATGEN="latgen-faster-mapped"
  LATGEN_THREADS_ARGS=()
fi

"${LATGEN}" "${LATGEN_THREADS_ARGS[@]}" \
  --beam=25.0 --max-active=5000 \
  --acoustic-scale=1.0 \
  --allow-partial=true \
  --determinize-lattice=false \
  --word-symbol-table="${SYMBOLS}" \
  "${WFST_HMM}" \
  "${GRAPH}" \
  "ark:${CONF_MAT}" \
  "ark,t:|gzip -c > ${LAT_GZ}" \
  "ark,t:${HYP_1BEST}"

gzip -t "${LAT_GZ}"

echo "=========================================="
echo "4. Labeling lattice edges with characters"
echo "=========================================="
zcat "${LAT_GZ}" | perl scripts/int2sym.pl -f 3 "${SYMBOLS}" > "${LAT_TXT}"

echo "=========================================="
echo "5. Converting Kaldi lattice to HTK SLF"
echo "=========================================="
gawk '
NF == 5 {
  w=$3
  if (w == "<blk>") w="<eps>"
  print $1, $2, w, $5
  next
}
NF == 4 && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ {
  w=$3
  if (w == "<blk>") w="<eps>"
  print $1, $2, w
  next
}
{ print }
' "${LAT_TXT}" > "${LAT_SLF_IN}"

mkdir -p "${SLF_OUT}"
perl scripts/convert_slf.pl "${LAT_SLF_IN}" "${SLF_OUT}"

echo "=========================================="
echo "6. Evaluating 1-best WER/CER"
echo "=========================================="
for f in "${TRAIN_VAL_DIR}"/*.txt; do
  id=$(basename "$f" .txt)
  printf "%s " "$id"
  tr '\n' ' ' < "$f"
  printf "\n"
done | sed 's/\s\s*/ /g' > "${REF_OUT}"

if awk 'NF > 1 { exit(($2 ~ /^[0-9]+$/) ? 0 : 1) }' "${HYP_1BEST}"; then
  perl scripts/int2sym.pl -f 2-200 "${SYMBOLS}" "${HYP_1BEST}" > "${HYP_SYM}"
else
  cp "${HYP_1BEST}" "${HYP_SYM}"
fi

gawk -v TEST_LIST="${REF_OUT}" '
BEGIN {
  while ((getline < TEST_LIST) > 0) DICT[$1] = $0
}
{
  if ($1 in DICT) {
    hyp=""
    for (i=2; i<=NF; i++) {
      if ($i == "<space>") hyp = hyp " "
      else hyp = hyp $i
    }

    ref=DICT[$1]
    sub("^[^ ]+ ", "", ref)

    print hyp "#" ref
  }
}' "${HYP_SYM}" > "${HYP2REF}"

if [ -x tasas/tasas ]; then
  echo -e "WER= "$(tasas/tasas -f "#" -s " " "${HYP2REF}")
  echo -e "CER= "$(tasas/tasas -f "#" "${HYP2REF}")
else
  echo "WARNING: tasas/tasas not found or not executable; skipping WER/CER."
fi

echo "=========================================="
echo "Finished successfully"
ls -lh "${CONF_MAT}" "${LAT_GZ}" "${HYP_1BEST}" "${LAT_TXT}" "${HYP2REF}" 2>/dev/null || true
echo "SLF files: $(find "${SLF_OUT}" -type f | wc -l)"
echo "=========================================="
