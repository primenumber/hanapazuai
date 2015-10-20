#!/bin/sh

for i in `seq 27 50`
do
  echo "${i}:"
  ./beam.sh `printf "%02d" $i`
done
