if [ $# -lt 1 ]; then
   echo "Usage ./mTopo.sh numModels"
   exit -1
fi

numModels=$1

i=1;
models=""
while [ $i -le $numModels ]; do
   models=$models" "$i
   i=$(($i+1))
done

cat <<EOF #> ${DIROUT}/topo
<Topology> 
<TopologyEntry> 
<ForPhones> 
${models}
</ForPhones> 
$(
  echo "<State> 0 <PdfClass>  0 <Transition> 1 1 </State>"
  echo "<State> 1 </State>"
)
</TopologyEntry> 
</Topology> 
EOF

