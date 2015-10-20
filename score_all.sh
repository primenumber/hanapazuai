#!/bin/sh

for i in `seq 1 50`
do
  NAME=`printf "%02d" $i`
  if [ -e output/${NAME}.out ]; then
    echo $NAME
    ./ai calc input/${NAME}.in < output/${NAME}.out
  fi
done
