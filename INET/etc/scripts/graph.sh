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
# Script to automate the graphing of many different combinations of plots for
# the eventual inclusion in latex.
# Example command line:
# echo "sh ~/scripts/graph.sh p 50 10 100"|batch -mv
# echo "sh ~/scripts/graph.sh p 50 3 30 &> ~/graph-pcoaf-fixed-internet-link-server-0.01.out" |batch -mv


# Do for loops of combinations that I want
# Create the omnetpp.ini for the combination that I want

# Create the XML that I want
# Run simulation. It segfaults at very end

# Process the omnetpp.vec and plot using modified gnuplot script to
# have the titles and axes I want inc. output eps filename

# rename omnetpp.vec to particular configuration .vec and move to datadir

# move eps files to datadir

# Create or/update an inclusion makefile for my latex setup so that those
# figures are also known as dependencies

# continue with next config


SHDIR=~/bash
# Directory to store plots and data obtained
DATADIR=~/src/phantasia/master/output
# Dir for ipv6suite sources (part of cps fuction too)
SOURCEDIR=~/src/IPv6Suite
SCRIPTDIR=~/scripts

# Retrieve bash cps functions and convnofastras
. $SHDIR/functions

#These dirs have to be manually created. Guess I could automate this step too. Leave that for later
L2DIR=l2
ODADDIR=odad
PLAINDIR=mip
HMIPDIR=$HMIPSUFFIX
#no such dir but placeholder and will be used to create FILENAME
FASTRAS=fastras
FASTBEACONS=beacon
WLAN=wlan
PCOAF=$PCOASUFFIX #pcoaf
ETHNET=eth
ETHDIR=$ETHNET

function run_sim
{
#$1 is $conf
    L2=l2trig
    ODAD=$ODADDIR
    PLAIN=$PLAINDIR
    HMIP=$HMIPDIR
    declare FILENAMES
    FILENAMES[0]=$L2
    FILENAMES[1]=$ODAD
    FILENAMES[2]=$PLAIN
    FILENAMES[3]=$HMIP
    FILENAMES[4]=$FASTRAS
    FILENAMES[5]=$FASTBEACONS
    FILENAMES[6]=$WLAN
    FILENAMES[7]=$PCOAF
    FILENAMES[8]=$ETHNET
    L2TIT="L2 Trigger"
    ODADTIT="Optimistic DAD"
    PLAINTIT="Mobile IPv6"
    FASTRASTIT="Fast Solicited RA"
    HMIPTIT="Hierarchical MIPv6"
    declare TITLES
    TITLES[0]=$L2TIT
    TITLES[1]=$ODADTIT
    TITLES[2]=$PLAINTIT
    TITLES[3]=$HMIPTIT
    TITLES[4]=$FASTRASTIT
    TITLES[5]="Fast RA beacons"
    TITLES[6]="IEEE802.11 WLAN"
    TITLES[7]="Previous Care of Address Forwarding"
    TITLES[8]="Ethernet network Regression test"
    NAMES="$L2DIR $ODADDIR $PLAINDIR $HMIPDIR $FASTRAS $FASTBEACONS $WLAN $PCOAF $ETHNET"

    FAST=MIPv6FastRANetwork
    BEACONS=$FAST-fastbeacons
#grep 'sim-time-limit =' omnetpp.ini |sed 's|sim-time-limit =||'|sed 's|s||'
    if echo $1|grep $FASTBEACONS &> /dev/null; then
	XMLFILE=$BEACONS
    else
	if [ "$XMLFILE" = "" ]; then
	    XMLFILE=$FAST
	fi
    fi
    if echo $1|grep $WLAN &>/dev/null ; then      
	XMLFILE=${EXDIRNAMES[1]}
	SIMTIMELIMIT="199.95"
    fi
    if [ "$SYNCCONFIG" = "t" ]; then
	cps $XMLFILE.xml
    fi
    if echo $1|grep $FASTRAS &>/dev/null ; then
	: 
    else 
	convnofast $XMLFILE
	XMLFILE=$XMLFILE-nofast
    fi
    if [ "$SYNCCONFIG" = "t" ]; then
	cps $INIFILE.ini
    fi
    #echo Building for configuration=$1
#FILENAME is used for filenames

    FILENAME=$BASENAME
    if echo $XMLFILE| grep PCOAF &> /dev/null; then
	if echo $XMLFILE|grep $PCOAF &> /dev/null; then
	    :
	else
	    #Needed to make filenames unique for pcoaForwardingNet from normal
	    #local AR sims
	    #FILENAME=nopcoaf
	    #Well same thing would happen for hmip too just have to live with
	    #fact that these sims should not be run at same time as the localAR ones
	    :
	fi
    fi
    GTITLENAME=
    i=0
    for name in $NAMES; do
	if echo $1|grep -e "\<$name\>" &>/dev/null; then
	    if [ "$FILENAME" != "" ] ; then
		FILENAME="$FILENAME-"
	    fi
	    if [ "$GTITLENAME" != "" ] ;then
		GTITLENAME="$GTITLENAME + "
	    fi
	    FILENAME=$FILENAME${FILENAMES[$i]}
	    GTITLENAME="$GTITLENAME${TITLES[$i]}"
	    FILENAMESER=$FILENAME
	fi
	 ((i++))
         #i=`expr $i + 1`
    done


    echo title is  $GTITLENAME
#    echo file is  $FILENAME
#    echo Running for configuration=$1

    if [ "$PARALLEL" = "p" ]; then
    #Make it unique for parallel and multiple runs
	local FILENAME_
	FILENAME_=${FILENAME}-$PPID-$HOSTNAME
	while [ -e ${FILENAME_}.ini ]; do
	    FILENAME=${FILENAME_}-`dd if=/dev/urandom bs=1c count=2 2> /dev/null|hexdump -d |awk '{print $2}'|grep  -e '[[:digit:]]'`
	    echo "Generating different filename $FILENAME"
	    if [ ! -e $FILENAME ]; then
		FILENAME_=$FILENAME
	    fi
	done
	FILENAME=$FILENAME_
    fi
    
    echo SIMTIMELIMIT is $SIMTIMELIMIT

    #echo "filename is $FILENAME"
    . $SCRIPTDIR/graph-omnetpp-runs.sh


    if [ "$FAIL" = "true" ]; then
	echo "FAILED at simulation $GTITLENAME of build $1"
	exit;
    fi

 echo "++++++++++++++++++completed+++++++++++"
 fn core\.\* |xargs rm || echo core files not removed
}

#----------------------MAIN------------------------
echo "$0 $*"
#CONFIGURATIONS="$PLAINDIR $L2DIR $ODADDIR $ODADDIR-$L2DIR"
#CONFIGURATIONS="$PLAINDIR"
CONFIGURATIONS="$ETHDIR"
#CONFIGURATIONS="$WLAN"
#can't run with normal AR local schemes sims
HMIP=$HMIPDIR
#CONFIGURATIONS="$HMIP $HMIP"
#CONFIGURATIONS="$PCOAF $PCOAF $HMIP $HMIP"
#CONFIGURATIONS="$PCOAF $PCOAF"
#Set to p to make it parallel
PARALLEL=$1
#Time to wait is TIMETORUN*CHECKTIME before considering run has gone wild and
#will be killed
TIMETORUN=$2
CHECKTIME=$3
#END RUN number
NUMBEROFRUNS=$4

BEGINSIMTIME=7 #Used by graph-plot-graph.sh
SIMTIMELIMIT="349.95" #set to diff value by each config

if [ $# -eq 5 ]; then
    BEGINRUNNUMBER=$5
    #TODO backup seed.txt and delete the first BEGINRUNNUMBER-1 seeds
else
    BEGINRUNNUMBER=0
fi

#DEBUG=y
DEBUG=
#copy config from ~/src/IPv6Suite?
SYNCCONFIG=t
if [ $# -eq 5 ]; then
    echo fifth arg is $5
    SYNCCONFIG=$5
fi

if [ "$TIMETORUN" = "" ]; then
    TIMETORUN=100
fi
if [ "$PARALLEL" = "" ]; then
    PARALLEL=s
fi
if [ "$CHECKTIME" = "" ]; then
    CHECKTIME=10
fi

#BINDIR=~/src/other/IPv6Suite\
BINDIR=~/src/IPv6Suite-cvsbuildtest/IPv6Suite
EXDIRNAMES[0]="MIPv6Network"
EXDIRNAMES[1]="WirelessEtherNetwork"
EXDIRNAMES[2]="HMIPv6Network"
EXDIRNAMES[3]="EthNetwork"

SIMDIR=Examples/${EXDIRNAMES[0]}
SIMRUN=3
SIMEXE=./${EXDIRNAMES[0]}
INIFILE=omnetpp
NETNAME=mipv6fastRANet

for conf in $CONFIGURATIONS; 
do
  TOPDIR=$BINDIR-$conf
  if [ "$conf" = "$PCOAF" ]; then
      NETNAME=pcoaForwardingNet
      if [ "$PCOAFAR" = "done" ]; then
	  TOPDIR=$BINDIR-$PLAINDIR
      else
	  TOPDIR=$BINDIR-$ODADDIR-$L2DIR
      fi
  fi
  if [ "$conf" = "$HMIP" ]; then
      NETNAME=hmipv6SimpleNet
      if [ "$HMIPAR" = "done" ]; then
	  TOPDIR=$BINDIR-$HMIPDIR
      else
	  TOPDIR=$BINDIR-$HMIPDIR-ar
      fi
  fi

  if [ "$conf" = "$ETHNET" ]; then
      NETNAME=ethernetwork
      #Run it when all abilities compiled in. Should not affect outcome
      TOPDIR=$BINDIR-all
      SIMDIR=Examples/${EXDIRNAMES[3]}
      INIFILE=omnetpp
      XMLFILE=EthNetwork
      SIMEXE=./${EXDIRNAMES[3]}
      SIMTIMELIMIT=101
      MAXHOSTS=50
      SIMRUN=1
      ruby <<END > $TOPDIR/$SIMDIR/random.txt
srand(0)
@@Iterations = $NUMBEROFRUNS
@@Hosts = $MAXHOSTS
randArray = Array.new
for i in 0...@@Iterations
  val = rand(@@Hosts)
  #Prevent 0 from occurring
  val = rand(@@Hosts) if val == 0 
  throw "Didn't expect 2 consecutive zeroes from rand" if val == 0
  randArray.push( val )
end

randArray.each {|x| \$stdout << x << "\n" }
      
END

  fi

  echo topdir is $TOPDIR 
  pushd $TOPDIR &>/dev/null
  if [ $? -ne 0 ]; then
      echo "Failed to change to $TOPDIR"
      continue;
  fi
#cmake -Dxx=ON $SOURCEDIR
  echo "building in $conf"
#cmake . &> junk.log && 
  make &>junk.log
  if [ $? -ne 0 ]; then
      echo "Failed to build in directory `pwd`"
      continue;
  fi
  make OutOfSourceCopyConfig &> /dev/null
  if [ "$SYNCCONFIG" = "t" ]; then
      pushd Etc;cps default.ini;popd
  fi
  if echo $conf|grep $WLAN &>/dev/null ; then
      SIMDIR=Examples/${EXDIRNAMES[1]}
      SIMRUN=3
      SIMEXE=./${EXDIRNAMES[1]}
  fi
  if echo $conf|grep $HMIP &>/dev/null ; then
      SIMDIR=Examples/${EXDIRNAMES[2]}
      SIMEXE=./${EXDIRNAMES[2]}
      INIFILE=HMIPv6Simple
      XMLFILE=$INIFILE
  fi
  if echo $conf|grep $PCOAF &>/dev/null ; then
      INIFILE=PCOAForwarding
      XMLFILE=$INIFILE
  fi

pushd $SIMDIR &> /dev/null

#do other mods to XML e.g. fast beacons or can do in run_sim
#only odad-l2 used akaroa

  if [ "$conf" != "$PCOAF" ] && [ "$conf" != "$HMIP" ]; then
      run_sim $conf
      if echo $conf|grep $PLAINDIR &> /dev/null; then
	  run_sim $conf-$FASTBEACONS
      fi
      run_sim $conf-$FASTRAS
  else
      SIMRUN=1
  fi

  if echo $conf|egrep "($HMIP|$PCOAF)" &>/dev/null ; then
      SIMTIMELIMIT="249.95"
  fi

  ARIMP=$ODADDIR-$L2DIR-$FASTRAS
if [ "$conf" = "$PCOAF" ]; then
    #For the new server side cbr ping
    SIMRUN=3
    BASENAME="PCOA"
    if [ "$PCOAFAR" != "done" ]; then
	run_sim $ARIMP
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $conf-$ARIMP
	PCOAFAR=done
    else
	XMLFILE=$INIFILE
	run_sim $PLAINDIR
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $conf
    fi
fi
if [ "$conf" = "$HMIP" ]; then
    BASENAME="HMIP"
    SIMRUN=3
    if [ "$HMIPAR" != "done" ]; then
	run_sim $ARIMP
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $PCOAF-$ARIMP
	
	XMLFILE=$INIFILE	
	convhmip $XMLFILE
	XMLFILE=$XMLFILE-$HMIP
	run_sim $conf-$ARIMP
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $conf-$ARIMP-$PCOAF
	HMIPAR=done
    else
	XMLFILE=$INIFILE
	run_sim $PLAINDIR
	convhmip $XMLFILE
	XMLFILE=$XMLFILE-$HMIP
	run_sim $conf
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $conf-$PCOAF

	XMLFILE=$INIFILE
	convpcoaf $XMLFILE
	XMLFILE=$XMLFILE-$PCOAF
	run_sim $PCOAF
    fi
fi
popd
popd
done


