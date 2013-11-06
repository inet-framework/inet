#!/bin/bash

#rm -r uj
mkdir uj
mkdir uj/results-elotte
mkdir uj/results

for i in results-elotte/*.sca
do
  ./cutScaHeader.pl <$i | ./sortScaFile.pl >uj/$i
done

for i in results/*.sca
do
  ./cutScaHeader.pl <$i | ./sortScaFile.pl >uj/$i
done
