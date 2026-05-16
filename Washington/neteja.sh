#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Use: ${0##*/} <List_test_files> <Path_Dir_Output> "
  exit 1
fi

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMGTXTENH_DIR="${SCRIPT_DIR}/imgtxtenh"
MAGICK6_RUNTIME="${IMGTXTENH_DIR}/runtime"

if [ -d "${MAGICK6_RUNTIME}/lib" ]; then
  export LD_LIBRARY_PATH="${MAGICK6_RUNTIME}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
  export MAGICK_CONFIGURE_PATH="${MAGICK6_RUNTIME}/etc/ImageMagick-6${MAGICK_CONFIGURE_PATH:+:${MAGICK_CONFIGURE_PATH}}"
  export MAGICK_CODER_MODULE_PATH="${MAGICK6_RUNTIME}/lib/ImageMagick-6.9.12/modules-Q16/coders${MAGICK_CODER_MODULE_PATH:+:${MAGICK_CODER_MODULE_PATH}}"
fi

DOUT="$2"

[ -e "$1" ] || { echo "$1 doesn't exist ..."; exit 1; }
[ -d "$DOUT" ] || mkdir -p "$DOUT"

while IFS= read -r file; do
   [ -n "$file" ] || continue
   echo "Processing $file"

   EXT="${file##*.}"
   N="$(basename "$file" ".$EXT")"
   NOM="${DOUT}/${N}"

  "${IMGTXTENH_DIR}/imgtxtenh"   -w 45 -s 0.2 -k 0.3  "$file"  "$NOM.jpg"

done < "$1"
