#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

IN="${IN:-results/lattices8/train_val-char-lat.slf-input}"
SHARDS="${SHARDS:-8}"
RUN_ID="${RUN_ID:-$(date +%Y%m%d_%H%M%S)}"
BASE="${BASE:-/tmp/${USER}/gw_slf_parallel_${RUN_ID}}"
WORK="${WORK:-${BASE}/shards}"
OUT="${OUT:-${BASE}/chars}"
LOG_DIR="${LOG_DIR:-logs}"

mkdir -p "${WORK}" "${OUT}" "${LOG_DIR}"

echo "=========================================="
echo "Parallel SLF conversion"
echo "Started: $(date)"
echo "Input: ${IN}"
echo "Shards: ${SHARDS}"
echo "Work dir: ${WORK}"
echo "Output dir: ${OUT}"
echo "=========================================="

if [ ! -s "${IN}" ]; then
  echo "ERROR: input not found or empty: ${IN}" >&2
  exit 1
fi

echo "Splitting input by complete lattices..."
awk -v shards="${SHARDS}" -v outdir="${WORK}" '
BEGIN {
  at_start = 1
  lattice = 0
  file = ""
}
{
  if (at_start && NF == 1) {
    shard = lattice % shards
    file = sprintf("%s/shard_%02d.lat", outdir, shard)
    lattice++
    at_start = 0
  }

  if (file != "") {
    print >> file
  }

  if (NF == 0) {
    if (file != "") close(file)
    file = ""
    at_start = 1
  }
}
END {
  if (file != "") {
    print "" >> file
    close(file)
  }
  printf("Split lattices: %d\n", lattice) > "/dev/stderr"
}
' "${IN}" 2> "${BASE}/split.log"
cat "${BASE}/split.log"

echo "Running ${SHARDS} convert_slf.pl workers..."
pids=()
for shard in "${WORK}"/shard_*.lat; do
  name="$(basename "${shard}" .lat)"
  (
    perl scripts/convert_slf.pl "${shard}" "${OUT}"
  ) > "${LOG_DIR}/parallel_convert_slf_${RUN_ID}_${name}.log" 2>&1 &
  pids+=("$!")
done

failed=0
for pid in "${pids[@]}"; do
  if ! wait "${pid}"; then
    failed=1
  fi
done

count="$(find "${OUT}" -maxdepth 1 -type f -name '*.lat.gz' | wc -l)"
echo "Finished: $(date)"
echo "Output files: ${count}"
echo "Output dir: ${OUT}"

if [ "${failed}" -ne 0 ]; then
  echo "ERROR: at least one worker failed. Check logs/parallel_convert_slf_${RUN_ID}_*.log" >&2
  exit 1
fi
