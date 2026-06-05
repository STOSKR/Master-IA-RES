#!/bin/bash
#SBATCH -p long
#SBATCH --job-name=washington-word-graphs
#SBATCH --cpus-per-task=10
#SBATCH --mem=32G
#SBATCH --gres=shard:1
#SBATCH --chdir=/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington
#SBATCH -o logs/%x-%j.log

set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/home/alumno.upv.es/scheng1/Master-IA-RES/GeorgeWashington}"
cd "${PROJECT_DIR}"

source /opt/miniconda3/etc/profile.d/conda.sh

mkdir -p data/data_train_val results/lattices/words models/Optical logs data/text

# ==========================================================
# Configuración inicial y rutas de los archivos
# ==========================================================
MODEL="models/Optical/washington_26.pth" # Adaptado a tu petición
TRAIN_DIR="data/data_train_80"
VAL_DIR="data/data_val_10"
TRAIN_VAL_DIR="data/data_train_val"

CONF_MAT="models/Optical/Conf_mats_train_val.ark"
WFST_HMM="models/WFST/model.hmm"
WFST_TLG="models/WFST/TLG.fst"
WORDS_MAP="models/WFST/wordsMap.txt"

LAT_GZ="results/lattices/train_val-word-lat.gz"
HYP_1BEST="results/lattices/train_val-3gram-words-1bestLat.hyp"
LAT_TXT="results/lattices/train_val-word-lat"
SLF_OUT="results/lattices/words"
REF_OUT="data/text/train_val_words.ref"
HYP2REF="results/lattices/train_val_WG_hyp2ref"

# ==========================================================
# 1. Getting the confidences matrix for rest corpus
# ==========================================================
echo "=========================================="
echo "1. Extrayendo la matriz de confianzas (PyTorch)"
echo "=========================================="

echo "Limpiando y copiando datos de $TRAIN_DIR y $VAL_DIR a $TRAIN_VAL_DIR..."
rm -rf "${TRAIN_VAL_DIR}"/* || true
cp -r "${TRAIN_DIR}"/* "${TRAIN_VAL_DIR}/" || true
cp -r "${VAL_DIR}"/* "${TRAIN_VAL_DIR}/" || true

# Activamos el entorno de entrenamiento (PyTorch) para el get_confMats
conda activate res_env

echo "Iniciando extracción con modelo: ${MODEL}"
python HTRnn/get_confMats_binary.py \
  --batch_size 4 \
  --conf_matrix "${CONF_MAT}" \
  "${MODEL}" \
  "${TRAIN_VAL_DIR}"


# ==========================================================
# 2. Get the word graphs
# ==========================================================
echo "=========================================="
echo "2. Generando los grafos de palabras (Kaldi)"
echo "=========================================="

# Cambiamos al entorno de Kaldi para los latgen y demás binariso
conda activate kaldi_env

# Usamos la versión paralela si existe, si no damos fallback al decodificador normal
LATGEN="latgen-faster-mapped-parallel"
if ! command -v "$LATGEN" >/dev/null 2>&1; then
  echo "Aviso: latgen-faster-mapped-parallel no encontrado, usando versión estándar."
  LATGEN="latgen-faster-mapped"
fi

$LATGEN --num-threads=10 \
  --beam=25.0 --max-active=5000 \
  --acoustic-scale=1.0 \
  "${WFST_HMM}" \
  "${WFST_TLG}" \
  "ark:${CONF_MAT}" \
  "ark,t:|gzip -c > ${LAT_GZ}" \
  "ark,t:${HYP_1BEST}"


# ==========================================================
# 3. Label the edges with words
# ==========================================================
echo "=========================================="
echo "3. Etiquetando los grafos de palabras"
echo "=========================================="
zcat "${LAT_GZ}" | perl scripts/int2sym.pl -f 3 "${WORDS_MAP}" > "${LAT_TXT}"


# ==========================================================
# 4. From Kaldi to HTK format
# ==========================================================
echo "=========================================="
echo "4. Convirtiendo a formato SLF de HTK"
echo "=========================================="
perl scripts/convert_slf.pl "${LAT_TXT}" "${SLF_OUT}"


# ==========================================================
# 5. Getting WER and CER of the 1_best on the word graphs
# ==========================================================
echo "=========================================="
echo "5. Calculando WER y CER"
echo "=========================================="

# Preparamos las referencias base extraídas del dataset original validado en paso 1
echo "Construyendo ref dict a partir de $TRAIN_VAL_DIR"
for f in "${TRAIN_VAL_DIR}"/*.txt; do
  id=$(basename "$f" .txt)
  printf "%s " "$id"
  tr '\n' ' ' < "$f"
  printf "\n"
done | sed 's/\s\s*/ /g' > "${REF_OUT}"

# Convertimos la salida de 1-best (ints) a símbolos y evaluamos
perl scripts/int2sym.pl -f 2-200 "${WORDS_MAP}" "${HYP_1BEST}" | \
gawk -v FILE_LIST="${REF_OUT}" 'BEGIN{
  while ((getline < FILE_LIST) > 0) DICT[$1] = $0
}{
  if ($1 in DICT) {
    N=split(DICT[$1],DIC)
    for (i=2; i<NF; i++) printf("%s ", $i)
    printf("%s#",$NF);
    for (i=2; i<N; i++) printf("%s ",DIC[i])
    printf("%s\n",DIC[N]);
  }
}' > "${HYP2REF}"

if [ -x tasas/tasas ]; then
  echo -e "WER= \t"$(tasas/tasas -f "#" -s " " "${HYP2REF}")
  echo -e "CER= \t"$(tasas/tasas -f "#" "${HYP2REF}")
else
  echo "ADVERTENCIA: tasas/tasas no se encontró o no tiene permisos de ejecución, omitimos la evaluacion WER/CER."
fi

echo "=========================================="
echo "Proceso Terminado Satisfactoriamente."
echo "=========================================="
