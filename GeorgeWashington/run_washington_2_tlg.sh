#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington2-tlg
#SBATCH --cpus-per-task=4
#SBATCH --mem=32G
#SBATCH --gres=shard:1
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh

mkdir -p logs models/Optical models/WFST results/washington_2_tlg data/text data/lists

MODEL="${MODEL:-models/Optical/washington_2.pth}"
TEST_DIR="${TEST_DIR:-data/data_test_10}"
TEST_LIST="${TEST_LIST:-data/lists/test_10.lst}"
SYMBOLS_IN="${SYMBOLS_IN:-data/lists/symbols_train.lst}"
SYMBOLS_TLG="${SYMBOLS_TLG:-data/lists/symbols_train_tlg.lst}"
TRAIN_WORDS="${TRAIN_WORDS:-data/text/train_words.txt}"
TRANSCRIPTIONS="${TRANSCRIPTIONS:-data/text/transcriptions.txt}"

CONF_MAT="${CONF_MAT:-models/Optical/Conf_mat_test_washington_2_tlg.ark}"
GRAPH="${GRAPH:-models/WFST/TLG.fst}"
WORDS_MAP="${WORDS_MAP:-models/WFST/wordsMap.txt}"
LEXICON="${LEXICON:-models/WFST/lexicon.txt}"
REF_OUT="${REF_OUT:-data/text/test_words_washington_2_tlg.ref}"

RESULTS_DIR="${RESULTS_DIR:-results/washington_2_tlg}"
HYP_NUM="${HYP_NUM:-${RESULTS_DIR}/test_TLG_numbers.hyp}"
HYP2REF="${HYP2REF:-${RESULTS_DIR}/test_TLG_hyp2ref}"

BATCH_SIZE="${BATCH_SIZE:-8}"
GPU_ID="${GPU_ID:-}"
HTR_GPU="${HTR_GPU:-0}"

RUN_CONF="${RUN_CONF:-1}"
RUN_MAKE_FST="${RUN_MAKE_FST:-1}"
RUN_DECODE="${RUN_DECODE:-1}"
RUN_TASAS="${RUN_TASAS:-1}"
RUN_EVAL="${RUN_EVAL:-1}"
ALLOW_INCOMPATIBLE_TLG="${ALLOW_INCOMPATIBLE_TLG:-0}"

SRILM_BIN="${SRILM_BIN:-/home/alumno.upv.es/scheng1/Master-IA-RES/Rodrigo/SRILM/bin/i686-m64}"

echo "=========================================="
echo "Washington_2 TLG pipeline"
echo "Project: ${PROJECT_DIR}"
echo "Model: ${MODEL}"
echo "Test dir: ${TEST_DIR}"
echo "Test list: ${TEST_LIST}"
echo "Confidence matrix: ${CONF_MAT}"
echo "Graph: ${GRAPH}"
echo "Results dir: ${RESULTS_DIR}"
echo "=========================================="

for path in "${MODEL}" "${TEST_DIR}" "${TEST_LIST}" "${SYMBOLS_IN}" "${TRAIN_WORDS}" "${TRANSCRIPTIONS}" "${WORDS_MAP}" "${LEXICON}"; do
  if [ ! -e "${path}" ]; then
    echo "ERROR: required input not found: ${path}" >&2
    exit 1
  fi
done

echo "=========================================="
echo "1. Getting the confidence matrix"
echo "=========================================="
if [ "${RUN_CONF}" = "1" ]; then
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
    "${TEST_DIR}"
else
  echo "Skipping confidence matrix extraction because RUN_CONF=${RUN_CONF}"
fi

if [ ! -s "${CONF_MAT}" ]; then
  echo "ERROR: confidence matrix not found or empty: ${CONF_MAT}" >&2
  exit 1
fi

echo "=========================================="
echo "2. Build T.fst, L.fst, G.fst and TLG.fst"
echo "=========================================="
if [ "${RUN_MAKE_FST}" = "1" ]; then
  conda activate kaldi_env
  export PATH="${SRILM_BIN}:${PATH}"

  sed 's/^~$/<space>/' "${SYMBOLS_IN}" > "${SYMBOLS_TLG}"

  awk 'NR==FNR { ok[$1]=1; next } BEGIN { ok["<blk>"]=1; ok["#0"]=1; ok["#1"]=1; ok["#2"]=1; ok["#3"]=1 } { for (i=2; i<=NF; i++) if (!($i in ok)) bad[$i]=1 } END { for (c in bad) print c }' \
    "${SYMBOLS_TLG}" "${LEXICON}" | sort > "${RESULTS_DIR}/missing_tlg_symbols.txt"

  if [ -s "${RESULTS_DIR}/missing_tlg_symbols.txt" ]; then
    echo "WARNING: the word lexicon contains symbols that are not in ${SYMBOLS_TLG}:"
    cat "${RESULTS_DIR}/missing_tlg_symbols.txt"
    if [ "${ALLOW_INCOMPATIBLE_TLG}" != "1" ]; then
      echo "ERROR: TLG cannot be built safely with this optical vocabulary."
      echo "Set ALLOW_INCOMPATIBLE_TLG=1 only if you want to reproduce the original failing command."
      exit 1
    fi
  fi

  AUX_TREE="/tmp/auxTree_${USER}_${SLURM_JOB_ID:-$$}" \
  LEXICON="${LEXICON}" \
  ./scripts/makeFST.sh 3 "${SYMBOLS_TLG}" "${TRAIN_WORDS}" "${CONF_MAT}"
else
  echo "Skipping makeFST because RUN_MAKE_FST=${RUN_MAKE_FST}"
fi

if [ ! -s "${GRAPH}" ]; then
  echo "ERROR: TLG graph not found or empty: ${GRAPH}" >&2
  exit 1
fi

echo "=========================================="
echo "3. Getting hypothesis using the TLG model"
echo "=========================================="
if [ "${RUN_DECODE}" = "1" ]; then
  conda activate kaldi_env
  decode-faster --print-args=false \
    --beam=30.0 --max-active=5000 \
    --acoustic-scale=1.0 --allow-partial=true \
    --word-symbol-table="${WORDS_MAP}" \
    "${GRAPH}" \
    "ark:${CONF_MAT}" \
    "ark,t:${HYP_NUM}"
else
  echo "Skipping decode because RUN_DECODE=${RUN_DECODE}"
fi

if [ ! -s "${HYP_NUM}" ]; then
  echo "ERROR: hypothesis file not found or empty: ${HYP_NUM}" >&2
  exit 1
fi

echo "=========================================="
echo "4. Getting the tasas evaluating tool"
echo "=========================================="
if [ "${RUN_TASAS}" = "1" ] && [ ! -x tasas/tasas ]; then
  wget --no-check-certificate http://www.prhlt.upv.es/~mpastorg/RES/tasas.tgz
  tar xvzf tasas.tgz
  make -C tasas tasas || (cd tasas && gcc -O3 -o tasas tasas.c)
else
  echo "tasas/tasas already available or RUN_TASAS=${RUN_TASAS}"
fi

echo "=========================================="
echo "5. Preparing the test references"
echo "=========================================="
gawk -v TEST_LIST="${TEST_LIST}" '
BEGIN {
  while ((getline < TEST_LIST) > 0) DICT[$1] = 1
}
{
  if ($1 in DICT) {
    printf("%s ", $1)
    $1=""
    N=split($0, CHAR, "")
    for (i=1; i<=N; i++) {
      printf("%s", CHAR[i])
    }
    printf("\n")
  }
}' "${TRANSCRIPTIONS}" | sed 's/\s\s*/ /g' > "${REF_OUT}"

echo "Reference lines: $(wc -l < "${REF_OUT}")"

echo "=========================================="
echo "6. Getting WER and CER"
echo "=========================================="
if [ "${RUN_EVAL}" = "1" ]; then
  scripts/int2sym.pl -f 2-200 "${WORDS_MAP}" "${HYP_NUM}" | \
  gawk -v TEST_LIST="${REF_OUT}" '
  BEGIN {
    while ((getline < TEST_LIST) > 0) DICT[$1] = $0
  }
  {
    if ($1 in DICT) {
      N=split(DICT[$1], DIC)
      for (i=2; i<NF; i++) printf("%s ", $i)
      printf("%s#", $NF)
      for (i=2; i<N; i++) printf("%s ", DIC[i])
      printf("%s\n", DIC[N])
    }
  }' > "${HYP2REF}"

  echo -e "WER= "$(tasas/tasas -f "#" -s " " "${HYP2REF}")
  echo -e "CER= "$(tasas/tasas -f "#" "${HYP2REF}")
else
  echo "Skipping evaluation because RUN_EVAL=${RUN_EVAL}"
fi

echo "=========================================="
echo "Finished Washington_2 TLG pipeline"
ls -lh "${CONF_MAT}" "${GRAPH}" "${HYP_NUM}" "${REF_OUT}" "${HYP2REF}" 2>/dev/null || true
echo "=========================================="
