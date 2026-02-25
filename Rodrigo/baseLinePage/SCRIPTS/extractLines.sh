#!/bin/bash

if [ ! $# -eq 3 ]; then
  echo "Use: ${0##*/} <testFileList> <Path_Dir_Input> <Path_Dir_Output>"
  exit 1
fi

#set -x : Display commands and their arguments as they are executed.
#set -v : Display shell input lines as they are read.
#set -n : Read commands but do not execute them. This may be used to check a shell script for syntax errors.
#set -e stop the script if an error occurs

PATH=$PATH:${HOME}/baseLinePage/BIN:
#DIN=${2/\//}
DIN=${2}
#DOUT=${3/\//}
DOUT=${3}
EXT=jpg

for file in `cat $1`; do
     Nom=`basename $file .$EXT`
     NOM_IN=`echo -e ${DIN}"/"${Nom}`
     NOM_OUT=`echo -e ${DOUT}"/"${Nom}`
     #echo -e $NOM" "`lineExtractor -x ${NOM}.xml -i ${NOM}.jpg -t -v 1`
     echo -e $NOM_IN" "`lineExtractor -x ${NOM_IN}.xml -i ${NOM_IN}.jpg -o ${NOM_OUT} -v 1`

done
