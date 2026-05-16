#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Use: ${0##*/} <List_test_files> <Path_Dir_Output> "
  exit 1
fi

set -e

DOUT=$2

[ -e $1 ] || { echo "$1 doesn't exist ..."; exit 1; }
[ -d $DOUT ] || mkdir $DOUT

for file in `cat $1`; do 
   echo "Processing $file"

   EXT=`head -1 $1 |awk -F "." '{print $NF}'`
   N=`basename $file .$EXT`
   NOM=`echo -e ${DOUT}"/"${N}`

  ./imgtxtenh/imgtxtenh   -w 45 -s 0.2 -k 0.3  $file  $NOM.jpg 

done

