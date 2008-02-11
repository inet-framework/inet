#!/bin/sh
# -*- Shell-script -*-
# Copyright (C) 2003 by Johnny Lai
#
# This script is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

#Run from directory of a simulation scenario with many runs as subdirs generated
#from graph.sh. Can be called from graph-collect if there are just too many runs to run this individually from each experiment dir

SIM=`basename $PWD`
DIRS=`find .  -type d -a ! \( -name  'bad*' \)`

#Names of output files 
HVEC=handover
DVEC=dropcount

#Thresholds to ignore run on for dropcount and handover latency resp.
DCTH=1000
HLTH=20

#Arrays
VECS=("$HVEC" "$DVEC")
#Header to satisfy plove
HEADERS=('vector 2  "mipv6fastRANet.client1.ping6App"  "handoverLatency"  1' 'vector 1  "mipv6fastRANet.client1.ping6App"  "pingDrop"  1')
THRESHS=($HLTH $DCTH)

#Collect stats from subdirectories and report suspicious values.
#These are removed only in fixStats
function collectStats
{
defects=0
local FAIL
FAIL=f

rm $HVEC.vec $DVEC.vec 

for d in $DIRS; 
do
  if [ "$d" = "." ]; then
      continue
  fi
  d=`echo $d|cut -b 3-` #erase ./
  if [ ! -e $d ]; then
      echo "Where is dir $d"
      exit
  fi

  local HTIME
  HTIME=`grep "^2\>" $d/$SIM-$d.vec | cut -f 3`
  for time in $HTIME; do
      timeInt=`echo $time|cut -d '.' -f 1`
      if [ $timeInt -gt $HLTH ]; then
	  echo "Run $d is stuffed it has handover of $time" 
	  #echo "Will remove suspicious values at end of run or just continue here and ignore that run"
	  FAIL=t
	  ((defects++))
      fi
  done

  local DCOUNT  
  DCOUNT=`grep "^1\>" $d/$SIM-$d.vec | cut -f 3`
  for dropc in $DCOUNT; do
      if [ $dropc -gt $DCTH ]; then
	  echo "Run $d Really stuffed dropcount of $dropc from long handover I guess"
	  #echo "Will remove at end or this whole run if not enough confidence or stuff that too"
	  FAIL=t
      fi
  done
  if [ "$FAIL" = "t" ]; then
      #echo "Skipping dud run $d"
      :
  fi

  grep "^2\>" $d/$SIM-$d.vec >> $HVEC.vec 
  grep "^1\>" $d/$SIM-$d.vec >> $DVEC.vec 
#/tmp/pipe1103-2 2>> /tmp/log1103-1.log
done
}

#Removes values that are past the threshold values from the 
function fixStats
{
local i
i=0
#for a in "${("$HVEC" "$DVEC")[@]}"; do
for v in "${VECS[@]}"; do
    cp -p $v.vec $v-orig.dat
#    sort -k 2 -n $v.vec > $v-sorted.dat
#    touch -r $v.vec $v-sorted.dat
#    cp -p $v-sorted.dat $v.vec

    local HEADER
    HEADER=${HEADERS[$i]}
    #remove bad lines
    #echo "Threshs is  ${THRESHS[$i]} for $v"
    sort -k 3 -n $v-orig.dat | awk "{ if (\$3 < ${THRESHS[$i]} ) print \$0 }" | sort -k 2 -n > $v.vec
    touch -r $v-orig.dat $v.vec
    cp -p $v.vec $v.dat
    echo $HEADER > tmp.vec
    cat $v.vec >> tmp.vec
    touch -r $v.vec tmp.vec
    mv tmp.vec $v.vec
    #echo "data := import::readdata("`pwd`/$v.dat","\t"):s:=stats::sample(data):stats::mean(s,3)stats::stdev(s,3):" >> mupad.txt
#    plot(plot::boxplot(s,3))

    ((i++))
done
if [ $defects -ne 0 ]; then
  echo "Recorded plove $HVEC.vec/$DVEC.vec and .dat for maths analysis with $defects defects removed"
fi
}

#Uses grace. Don't like it because histogram is different to both R and mathematica which agree
function graphStats()
{
    #grace only accepts commands from stdin not data
    #awk '{print $2, $3}' < handover.dat |xmgrace -dpipe 0
    #page size is A5
    
    for v in "${VECS[@]}"; do
	#Max never worked
	#xmgrace -block $v.dat -settype xy -bxy 2:3 -hdevice EPS  -pexec "page size 792, 612;histogram(s0,mesh(0,MAX(s0.y),MAX(s0.y)/0.05),off,off)" -hardcopy -printfile $v.eps
	#gracebat -block $v.dat -settype xy -bxy 2:3 -hdevice EPS  -pexec "page size 792, 612;histogram(s0,mesh(0,$max,$max/$bin),off,off);kill s0;autoscale onread xyaxes" -hardcopy -printfile $v.eps
	if echo $v|grep hand; then	    
	    gracebat -hdevice EPS -hardcopy -printfile $v.eps -block $v.dat -settype xy -bxy 2:3 -batch ~/scripts/grace-hist.bat -pexec 'xaxis  ticklabel append "s";legend off;s1 legend "Latency"; subtitle "Mobile IPv6"; title "Handover Latency"; xaxis label "Handover Latency"' -pexec 'histogram(s0,mesh(0,4,20),off,off); kill s0;autoscale'
	else
	    gracebat -hdevice EPS -hardcopy -printfile $v.eps -block $v.dat -settype xy -bxy 2:3 -batch ~/scripts/grace-hist.bat -pexec 'legend off; s1 legend "Drop Count"; subtitle "Mobile IPv6"; title "Handover Packet Loss"; xaxis label "Packet Drop Count"; histogram(s0,mesh(0,70,20),off,off); kill s0;autoscale'
	fi
    done
}

function main
{
    collectStats
    fixStats
#    graphStats
}

main
#Non nested to treat all values as from same population or sample
###data := import::readdata("`pwd`/$HVEC.vec", NonNested)
#if data is more than 1 column i.e. 1 column per sampled variable or simulation
###s:=stats::sample(data)

###stats::mean(data)
###stats::stdev(data)
