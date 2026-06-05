#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington-decode-tg
#SBATCH --cpus-per-task=1
#SBATCH --mem=32G
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh
conda activate kaldi_env

mkdir -p logs results results/lattices8

BEAM="${BEAM:-25.0}"
MAX_ACTIVE="${MAX_ACTIVE:-5000}"
ACOUSTIC_SCALE="${ACOUSTIC_SCALE:-1.0}"
RUN_DECODE="${RUN_DECODE:-1}"
RUN_POSTPROCESS="${RUN_POSTPROCESS:-1}"
RUN_LAT_CONVERT="${RUN_LAT_CONVERT:-1}"
RUN_SLF_CONVERT="${RUN_SLF_CONVERT:-1}"

MODEL="${MODEL:-models/WFST/model.hmm}"
GRAPH="${GRAPH:-models/WFST/TG.fst}"
FEATS="${FEATS:-models/Optical/Conf_mat_test.ark}"
SYMBOLS="${SYMBOLS:-models/WFST/tokensMap.txt}"
LAT_OUT="${LAT_OUT:-results/lattices8/test-char-lat.gz}"
HYP_OUT="${HYP_OUT:-results/test_TG_chars.hyp}"
LAT_TXT="${LAT_TXT:-results/lattices8/test-char-lat}"
LAT_SLF_IN="${LAT_SLF_IN:-results/lattices8/test-char-lat.slf-input}"
SLF_DIR="${SLF_DIR:-results/lattices8/chars}"
TEST_DIR="${TEST_DIR:-data/debug_eval_40}"
REF_OUT="${REF_OUT:-data/text/test_words.ref}"
HYP2REF_OUT="${HYP2REF_OUT:-results/test_TG_hyp2ref}"

JOB_TAG="${SLURM_JOB_ID:-$$}"
LAT_TMP="${LAT_OUT}.partial.${JOB_TAG}"
HYP_TMP="${HYP_OUT}.partial.${JOB_TAG}"

echo "=========================================="
echo "Starting Washington TG char decoding: $(date)"
echo "Project: ${PROJECT_DIR}"
echo "Job id: ${JOB_TAG}"
echo "Graph: ${GRAPH}"
echo "Features: ${FEATS}"
echo "Symbols: ${SYMBOLS}"
echo "Lattice output: ${LAT_OUT}"
echo "Hypothesis output: ${HYP_OUT}"
echo "Postprocess: ${RUN_POSTPROCESS}"
echo "Lattice id-to-symbol conversion: ${RUN_LAT_CONVERT}"
echo "SLF conversion: ${RUN_SLF_CONVERT}"
echo "=========================================="

for path in "${MODEL}" "${GRAPH}" "${FEATS}" "${SYMBOLS}"; do
  if [ ! -s "${path}" ]; then
    echo "ERROR: required input not found or empty: ${path}" >&2
    exit 1
  fi
done

if [ "${RUN_DECODE}" = "1" ]; then
  rm -f "${LAT_TMP}" "${HYP_TMP}"

  latgen-faster-mapped --print-args=false \
    --beam="${BEAM}" --max-active="${MAX_ACTIVE}" \
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
else
  echo "Skipping decode because RUN_DECODE=${RUN_DECODE}"
fi

if [ "${RUN_POSTPROCESS}" = "1" ]; then
  for path in "${LAT_OUT}" "${HYP_OUT}"; do
    if [ ! -s "${path}" ]; then
      echo "ERROR: required decode output not found or empty: ${path}" >&2
      exit 1
    fi
  done

  if [ "${RUN_LAT_CONVERT}" = "1" ]; then
    echo "Converting character lattice ids to symbols..."
    if [[ "${LAT_OUT}" == *.gz ]]; then
      gzip -t "${LAT_OUT}"
      zcat "${LAT_OUT}" | perl scripts/int2sym.pl -f 3 "${SYMBOLS}" > "${LAT_TXT}"
    else
      perl scripts/int2sym.pl -f 3 "${SYMBOLS}" "${LAT_OUT}" > "${LAT_TXT}"
    fi
  else
    echo "Skipping lattice id-to-symbol conversion because RUN_LAT_CONVERT=${RUN_LAT_CONVERT}"
    if [ ! -s "${LAT_TXT}" ]; then
      echo "ERROR: LAT_TXT is missing or empty: ${LAT_TXT}" >&2
      exit 1
    fi
  fi

  if [ "${RUN_SLF_CONVERT}" = "1" ]; then
    echo "Preparing character lattice for SLF conversion..."
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

    echo "Converting character lattice to SLF..."
    mkdir -p "${SLF_DIR}"
    perl scripts/convert_slf.pl "${LAT_SLF_IN}" "${SLF_DIR}"
  else
    echo "Skipping SLF conversion because RUN_SLF_CONVERT=${RUN_SLF_CONVERT}"
  fi

  if [ -d "${TEST_DIR}" ]; then
    echo "Building reference file from ${TEST_DIR}..."
    mkdir -p "$(dirname "${REF_OUT}")"
    for f in "${TEST_DIR}"/*.txt; do
      id=$(basename "$f" .txt)
      printf "%s " "$id"
      tr '\n' ' ' < "$f"
      printf "\n"
    done | sed 's/\s\s*/ /g' > "${REF_OUT}"
  elif [ ! -s "${REF_OUT}" ]; then
    echo "ERROR: reference file is missing and TEST_DIR is not available: ${REF_OUT}" >&2
    exit 1
  fi

  echo "Building hyp2ref..."
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
  }' "${HYP_OUT}" > "${HYP2REF_OUT}"

  if [ -x tasas/tasas ]; then
    echo -e "WER= "$(tasas/tasas -f "#" -s " " "${HYP2REF_OUT}")
    echo -e "CER= "$(tasas/tasas -f "#" "${HYP2REF_OUT}")
  else
    echo "WARNING: tasas/tasas not found or not executable; skipping WER/CER."
  fi
fi

echo "=========================================="
echo "Finished Washington TG char decoding: $(date)"
ls -lh "${LAT_OUT}" "${HYP_OUT}" 2>/dev/null || true
[ -s "${LAT_TXT}" ] && ls -lh "${LAT_TXT}" || true
[ -d "${SLF_DIR}" ] && echo "SLF files: $(find "${SLF_DIR}" -type f | wc -l)" || true
echo "Hypothesis lines: $(wc -l < "${HYP_OUT}")"
echo "=========================================="
