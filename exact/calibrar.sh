#!/bin/bash
# Uso: ./calibrar2.sh
MIN=480; MAX=720; LIMITE=14
SEEDS=12
C=10; L=8; A=8

# "minDim maxDim" : caixas menores => busca mais profunda
for dims in "3 6" "2 5" "2 4" "2 6"; do
  set -- $dims; dmin=$1; dmax=$2
  for n in 24 28 32 36 40 45; do
    for seed in $(seq 1 $SEEDS); do
      nome="f${dmin}${dmax}_${n}_${seed}"
      ini=$(date +%s)
      saida=$(./build/test_branch_and_bound $nome $n 1 $dmin $dmax $C $L $A $LIMITE)
      secs=$(( $(date +%s) - ini ))
      estado=$(echo "$saida" | grep -E "Otimo|TEMPO")
      razao=$(echo "$saida" | grep -o "([0-9.]*x o container)")
      echo "dims=$dmin-$dmax n=$n seed=$seed ${secs}s $razao $estado"
      if [ $secs -ge $MIN ] && [ $secs -le $MAX ] && echo "$estado" | grep -q "Otimo"; then
        echo ">>> Escolhida: test/instances/$nome"; exit 0
      fi
    done
  done
done