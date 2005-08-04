#! /bin/sh
# Copyright (C) 2003 by Johnny Lai
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#Script to be sourced into main script. Otherwise will be missing some values
#SIMEXE=$1
#FILENAME=$2
#SIMRUN=$3
#NUMBEROFRUNS=$4
#BEGINRUNNUMBER=$5 or 0
local NUMBEROFGOODRUNS
NUMBEROFGOODRUNS=0

function iterate
{
    echo "Simulation Run ${i}..."
    local BADRUN
    BADRUN=false
    local FILENAMEI
    if [ "$PARALLEL" = "p" ]; then
        FILENAMEI=$FILENAME-$i
    else
        FILENAMEI=$FILENAME
    fi
    cp -p $INIFILE.ini $FILENAMEI.ini
#remove default.ini because that contains a dummy xml file and ini is law
#for first entry
    perl -i -pwe 's|.*\.xml\"$||g' $FILENAMEI.ini
    perl -i -pwe 's|.*default.ini$||g' $FILENAMEI.ini
    #standardize networkname
    echo "$NETNAME.*.IPv6routingFile =\"$XMLFILE-$FILENAMEI.xml\"" >> $FILENAMEI.ini
    echo "include ../../Etc/default.ini" >> $FILENAMEI.ini
    if [ "$PARALLEL" = "p" ]; then
        echo "include params.ini" >> $FILENAMEI.ini
    fi
    if [ "$NETNAME" = "ethernetwork" ]; then
        ruby << END > tmp
        File.open("random.txt", "r") do |file|
        count = 0
        file.each_line do |line|
        puts line if count == ${i}
        count+=1
        end
        end
END
        echo ethernetwork.numOfClients = `cat tmp` >> $FILENAMEI.ini
        rm tmp
    fi
    CWDEBUGFILE=debug-${FILENAMEI}.log
    cp -p $XMLFILE.xml $XMLFILE-$FILENAMEI.xml
    perl -i -pwe "s|debug.\.log|${CWDEBUGFILE}|" $XMLFILE-$FILENAMEI.xml
    perl -i -pwe "s|debug\.log|${CWDEBUGFILE}|" $XMLFILE-$FILENAMEI.xml
    echo $SIMEXE -f $FILENAMEI.ini -r $SIMRUN '&>' test-$FILENAMEI.out
    if [ "$DEBUG" != "y" ]; then
        $SIMEXE -f $FILENAMEI.ini -r $SIMRUN &> test-$FILENAMEI.out &
    fi
    #do some checks on output e.g. ended etc.
    j=0
    until grep "Calling finish" test-$FILENAMEI.out &> /dev/null
      do
      sleep $CHECKTIME
      if ps -ef|grep $SIMEXE|grep $USER|grep -v grep &> /dev/null; then
          if [ $j -gt $TIMETORUN ]; then
             # echo "Exceedingly long run $((TIMETORUN*10))"
	      echo "Exceedingly long run $((TIMETORUN*CHECKTIME))"
              break;
          fi
      else
          if grep "Calling finish" test-$FILENAMEI.out &> /dev/null; then
              :
          else
              echo "Run $i DIED prematurely"
              BADRUN=true
              if [ "$PARALLEL" = "p" ]; then
                  break;
              else
                  return 2;
              fi
          #continue;
          fi
      fi
      ((j++))
    done
    #Give it some time to finish (usually takes a long time like forever)
    sleep $CHECKTIME
    if killall $SIMEXE &>/dev/null; then
        echo "had to kill!"
    fi
    if ps -ef|grep $SIMEXE|grep $USER|grep -v grep &> /dev/null; then
        killall -9 $SIMEXE &> /dev/null
        echo "brute force required to kill!!"
    fi
    if [ -f omnetpp.vec ]; then
        local value
        value=`tail -1 < omnetpp.vec | cut -f 2`    
        #if [ $value -lt $SIMTIMELIMIT ]; then #works for integers only
        value=`echo $value '<' $SIMTIMELIMIT|bc`
        if [ $value -eq 1 ] ; then
            echo "Run $i DID not finish properly"
            BADRUN=true
#       exit
        fi
    else
        echo "Really horrible run $i as no ping packet received"
        BADRUN=true
    fi
    #Fix previous versions incorrect assesment of bad runs
    if false; then
        local dirs
        local outdirs
        dirs=`fn \*.vec`
        for d in $dirs
          do 
          value=`tail -1 < $d | cut -f 2`
          value=`echo $value '>' $SIMTIMELIMIT|bc`
          if [ $value -eq 1 ] ; then
              outdirs="$outdirs `dirname $d`"
          fi
        done
        echo corrected dirs are $outdirs
        rename bad- "" $outdirs

        #Spot anomalies in bad runs (not caused by ethnetwork output queue problem)
        #number of lines should match
        grep -il "Simulation time limit" `fn \*.out`|grep bad|wc
        fn bad\*|wc
    fi
    bzip2 ${CWDEBUGFILE}
    bzip2 test-$FILENAMEI.out
    mv omnetpp.vec $FILENAMEI.vec
    if [ "$DEBUG" != "y" ]; then
        . $SCRIPTDIR/graph-plot-graph.sh
    fi

    DESTDIR=$DATADIR/$FILENAME
    
    if [ "$PARALLEL" = "p" ]; then
        if [ "$BADRUN" != "true" ]; then
            DESTDIR=$DESTDIR/$i
            ((NUMBEROFGOODRUNS++))
        else
            DESTDIR=$DESTDIR/bad-$i    
        fi
    else
        DESTDIR=$DESTDIR-single
    fi
    mkdir -pv $DESTDIR
    mv params.ini *.sh $FILENAMEI.vec $FILENAMEI.ini *.bz2 *.out $XMLFILE-$FILENAMEI.xml $DESTDIR

    if [ "$PARALLEL" != "p" ]; then
        mv *.${GFORMAT} *.${GFORMAT}c $DESTDIR
    fi
    echo "Results moved to $DESTDIR"

    if [ "$NUMBEROFGOODRUNS" = "$NUMBEROFRUNS" ]; then
        break
    fi
#grep  -e "^[012]x[[:digit:]]" test-$FILENAMEI.out > result-$FILENAMEI.out
#HANDOVER_TIMES=`grep MobileMove debug.log|grep -e "client1 [[:digit:]].*"|cut -d ' ' -f 9`
#echo $HANDOVER_TIMES > handover_$i.out
#awk -f ~/scripts/ipv6suite-find-handover.awk -v "time=$HANDOVER_TIMES" -- result-$FILENAMEI.out >> handover-$FILENAMEI.out 
#i=`expr $i + 1`

}

function execute_runs
{
    if [ "$PARALLEL" = "p" ]; then
        if [ ! -e seeds.txt ]; then
            seedtool g 983753 100000000 $NUMBEROFRUNS > seeds.txt
	    cp -p seeds.{txt,orig}
        else
            if [ $BEGINRUNNUMBER -eq 0 ]; then
                cp -p seeds.orig seeds.txt
                echo "sync with original seeds.txt"
            else
                echo "Warning maybe remove `pwd`/seeds.txt if you want diff number of runs"
            fi
#    return
            :
        fi
        local i
        #Change the starting run number if you are resuming interupted runs and
        #delete the correct number of seeds from seeds.txt
        i=$BEGINRUNNUMBER
        for seed in `cat seeds.txt`; do
            (
                echo "[General]"
                echo "random-seed = ${seed}"
            ) > params.ini
            iterate 
            if [ "$i" = "$NUMBEROFRUNS" ]; then
                break;
            fi
            ((i++))
        done
    else
        NUMBEROFRUNS=1
        i=0
        iterate
    fi
}

FAIL=true
execute_runs
FAIL=false
