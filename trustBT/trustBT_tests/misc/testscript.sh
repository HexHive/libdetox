#!/bin/sh

export LD_PRELOAD=/usr/lib/libfastbt.so

prog[1]="ooffice"
prog[2]="oocalc"
prog[3]="kcalc"
prog[4]="gvim"
prog[5]="gimp"
prog[6]="perl perlscript.pl"

if [ -e "testscript.log" ]
then
  rm testscript.log
fi

for index in 1 2 3 4 5 6
do
  echo -e "\n [${prog[index]}]\n\n" >> testscript.log
  ${prog[index]}  >> testscript.log 2>&1
  echo -e "\n exit status of ${prog[index]} = $?\n\n" >> testscript.log
done


