if [ $# -lt 4 ]; then
    echo  "Usage: makeWFST.sh N trainSimbols.list transcriptionsFile ConfMatrix.ark"
    echo  "    N -> integer (N-Gram)"
    echo  "    trainSimbols.list -> plain text file list, one symbol per line"
    echo  "    transcriptionsFile -> plain text file with char level transcriptions"
    echo  "    ConfMatrix.ark -> file containign the confidence matrix"
    exit -1
fi

N=$1
TRAIN_SYMBOLS=$2
TRAIN_TEXT=$3
CONF_MAT=$4

[ -e models/WFST ] || mkdir -p models/WFST

################################################################
### T.fst
################################################################
awk 'BEGIN{
  cont=0;
  print "<eps>",cont++;
  print "<ctc>",cont++;
  print "<blk>",cont++; getline; getline;
}{ print $1,cont++; }
END{
  print "#0",cont++;
  print "#1",cont++;
  print "#2",cont++;
  print "#3",cont++;
}' $TRAIN_SYMBOLS > models/WFST/tokensMap.txt


scripts/ctc_token_fst.py models/WFST/tokensMap.txt | \
  fstcompile --isymbols=models/WFST/tokensMap.txt --osymbols=models/WFST/tokensMap.txt | \
  fstarcsort --sort_type=olabel > models/WFST/T.fst


[ -e models/LM ] || mkdir -p models/LM

if [ $N -gt 3 ];
then
   ################################################################
   ### G.fst
   ################################################################
   
   cut -d" " -f2- ${TRAIN_TEXT} |ngram-count -text -  -order $N -wbdiscount1 -kndiscount -interpolate -lm models/LM/${N}-gram-chars.arpa

   arpa2fst models/LM/${N}-gram-chars.arpa | fstprint | sed -e 's/<eps>/\#0/' -e 's/<s>/<eps>/g' \
   	-e 's/<\/s>/<eps>/g' |fstcompile --isymbols=models/WFST/tokensMap.txt \
	--osymbols=models/WFST/tokensMap.txt | fstrmepsilon | \
	fstarcsort --sort_type=ilabel > models/WFST/G.fst
	
   ################################################################
   ### TG.fst
   ################################################################

   fsttablecompose models/WFST/T.fst models/WFST/G.fst  > models/WFST/TG.fst

else

   ################################################################
   ### TLG.fst
   ################################################################
   ## G.fst
   ngram-count -order $N -kndiscount -interpolate \
        -text  ${TRAIN_TEXT} -lm models/LM/3-gram-words.arpa

#   cut -d" " -f2- ${TRAIN_TEXT} |ngram-count -text -  -order $N -wbdiscount1 -kndiscount -interpolate -lm models/LM/${N}-gram-chars.arpa

   arpa2fst models/LM/${N}-gram-words.arpa | fstprint | sed -e 's/<eps>/\#0/' -e 's/<s>/<eps>/g' \
        -e 's/<\/s>/<eps>/g' |fstcompile --isymbols=models/WFST/wordsMap.txt \
        --osymbols=models/WFST/wordsMap.txt | fstrmepsilon | \
        fstarcsort --sort_type=ilabel > models/WFST/G.fst

   ## L.fst ## 
   token_disamb=`grep \#0 models/WFST/tokensMap.txt| awk '{print $2}'`
   word_disamb=`grep \#0 models/WFST/wordsMap.txt| awk '{print $2}'`

   scripts/make_lexicon_fst.pl \
	models/WFST/lexicon.txt 0.5 "<space>" '#'3| \
	fstcompile --isymbols=models/WFST/tokensMap.txt \
	-osymbols=models/WFST/wordsMap.txt | \
	fstaddselfloops "echo $token_disamb |" "echo $word_disamb |"| \
	fstarcsort --sort_type=olabel > models/WFST/L.fst

   ### LG.fst
   fsttablecompose models/WFST/L.fst models/WFST/G.fst |\
   fstdeterminizestar --use-log=true |\
   fstminimizeencoded |\
   fstarcsort --sort_type=ilabel > models/WFST/LG.fst

   fsttablecompose models/WFST/T.fst models/WFST/LG.fst > models/WFST/TLG.fst
fi
################################################################
### HMM
################################################################

#N_FEAT=`feat-to-dim ark:models/Optical/ConfMats.ark -`
N_FEAT=`feat-to-dim ark:${CONF_MAT} -`
./scripts/mkTopo.sh ${N_FEAT} > models/WFST/topology.hmm

gmm-init-mono --print-args=false models/WFST/topology.hmm \
${N_FEAT} models/WFST/model.hmm models/WFST/auxTree
