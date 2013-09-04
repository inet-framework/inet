#!/bin/bash
ERROR=0
while read SIM;do
echo "$SIM"
#Ejecutar la simulacion $SIM  Si genera mucha basura copiar a un result interno
mkdir -p results
opp_run -l ../../../out/gcc-debug/src/libinet.so -n ../../:../../../src/ -u Cmdenv -c $SIM >> results/SimOutput
if [ -e "results/SimOutput" ]; then
	mkdir -p $SIM/results
	mv -f results/* $SIM/results/
	ls $SIM/References/ | ./Compare.sh $SIM
	if [ $? = 1 ]; then
		ERROR=1
	fi
else
	echo "...........[FAIL]"
	ERROR=1
fi
done;
exit $ERROR
