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
# Run from graph.sh or graph-omnetpp-runs.sh as it requires quite a few
# variables from them

#Draws the axises titles and the commands for use in plot-graph. Requires
#graph.sh for the GTILENAME
function plotvector
{
#$1 is the type of vector to plot 1 is for eed and 2 is the combined one
    local PLOTTYPE
    PLOTTYPE=("impulses" "linespoints")
    local LEGENDS
    LEGENDS=("pingDelay" "pingDrop" "handoverLatency")
    local NENAME
    NENAME="mipv6fastRANet.client1.ping6App ($FILENAME_.vec)"
    local PIPENAME
    PIPENAME=/tmp/pipe$PPID
    local GRFRACTION
#GFRACTION is the columns to plot
    GFRACTION="2:3"
    local LOGNAME
    LOGNAME=/tmp/log$PPID-$1.log

    local COMMAND_
    COMMAND_="\"$PIPENAME-$1\" using $GFRACTION title \"${LEGENDS[$1]} in $NENAME\" with ${PLOTTYPE[$1]}"
    if [ "$1" = "1" ]; then    
	COMMAND="$COMMAND_, \"$PIPENAME-$((i+1))\" using $GFRACTION title \"${LEGENDS[$((i+1))]} in $NENAME\" with ${PLOTTYPE[$1]}"
    else 
	COMMAND=$COMMAND_
    fi
    CLEANPIPE="rm -f $PIPENAME-$1; mknod $PIPENAME-$1 p"
    if [ "$1" = "1" ]; then
	CLEANPIPE="$CLEANPIPE; rm -f $PIPENAME-$((i+1)); mknod $PIPENAME-$((i+1)) p"
    fi

    #CREATEPIPE="grep \"^$1\>\" \"$PWD/$FILENAME_.vec\" > $PIPENAME-$1 2>> $LOGNAME &"
    CREATEPIPE="grep \"^$1\>\" \"$FILENAME_.vec\" > $PIPENAME-$1 2>> $LOGNAME &"
    if  [ "$1" = "1" ]; then
    #for some weird reason the command sleep 10&; sleep 10 & works in interactive but scripts do not allow the \; operator
	CREATEPIPE="$CREATEPIPE grep \"^$((i+1))\>\" \"$FILENAME_.vec\" > $PIPENAME-$((i+1)) 2>> $LOGNAME &"
    fi
}

function plotgraph
{
    local FILENAME_
    
    if [ "$PARALLEL" != "p" ]; then
	FILENAME_=$FILENAME
    else       
	FILENAME_=$FILENAMEI
    fi
    
    #echo "Generating the scripts for gnuplot"
#Plotting Plottypes
SUFFIXES=("eed" "comb")
declare GYAXISNAMES
GYAXISNAMES=("End to End Ping Delay (s)" "Drop Count and Handover Latency (s)")
local i
i=0
for plottype in "${SUFFIXES[@]}"; do
    plotvector $i
    GSCRIPTNAME=plot-$FILENAME_-${SUFFIXES[$i]}.sh
    #OUTPUTPLOT="$PWD/plot-$FILENAME_-${SUFFIXES[$i]}"
    OUTPUTPLOT="plot-$FILENAMESER-${SUFFIXES[$i]}"
    GFORMAT=eps
    CAPTION="Plot of ${GYAXISNAMES[$i]} for $GTITLENAME"
    cat << EOF > $GSCRIPTNAME
#echo "Plotting Plot type=$plottype"
CAPTION="Plot of ${GYAXISNAMES[$i]} for $GTITLENAME"
echo "$CAPTION"
#sed (s) and units from axis don'tk now how though
#if echo $conf|grep $PLAINDIR &> /dev/null; then
# add $PLAINTIT to just after word for
#fi

$CLEANPIPE

$CREATEPIPE
echo "Creating ${OUTPUTPLOT}.${GFORMAT}c"
gnuplot << END
## uncomment this for file output:
    set terminal postscript ${GFORMAT} color
    set output "${OUTPUTPLOT}.${GFORMAT}c"
    set title "$GTITLENAME"
    set xlabel "Time (s)"
    set ylabel "${GYAXISNAMES[$i]}"
    set xrange [$BEGINSIMTIME:$SIMTIMELIMIT]
# place for gnuplot commands like:
# set logscale y
# set noxtics
# set tics out
# set arrow to 10,2

    plot $COMMAND
##echo "${OUTPUTPLOT}.${GFORMAT}c created"
END

$CLEANPIPE

$CREATEPIPE
echo "Creating ${OUTPUTPLOT}.${GFORMAT}"
gnuplot << END
    set terminal postscript ${GFORMAT}
#produce eps bw as colour uses green 
    set output "${OUTPUTPLOT}.${GFORMAT}"
    set title "$GTITLENAME"
    set xlabel "Time (s)"
    set ylabel "${GYAXISNAMES[$i]}"
    set xrange [$BEGINSIMTIME:$SIMTIMELIMIT]
    plot $COMMAND
##echo "${OUTPUTPLOT}.${GFORMAT} created"
END
EOF


    if [ "$PARALLEL" != "p" ]; then
	    #echo Calling script $GSCRIPTNAME
	sh $GSCRIPTNAME
	:
    fi
    ((i++))
done
#end plotting of gnuplot


}

plotgraph
