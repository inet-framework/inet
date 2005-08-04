

############Main
SHDIR=~/bash
# Directory to store plots and output data obtained under subdir $FILENAME
DATADIR=~/src/phantasia/master/output
# Dir for ipv6suite sources (part of cps fuction too)
#SOURCEDIR=~/src/IPv6Suite
SCRIPTDIR=~/scripts

# Retrieve bash cps functions and convnofastras
. $SHDIR/functions


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
#Sleeps this many seconds before checking if sim has finished either by grepping
#for finishing line or as stated above (see graph-omnetpp-runs.sh)
CHECKTIME=$3
#END RUN number
NUMBEROFRUNS=$4

if [ $# -eq 5 ]; then
    BEGINRUNNUMBER=$5
    #TODO backup seed.txt and delete the first BEGINRUNNUMBER-1 seeds
else
    BEGINRUNNUMBER=0
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

SRCDIR=${SOURCEDIR}
if [ ! -d "${SRCDIR}" ]; then
  echo "Please set SRCDIR i.e. where IPv6Suite sources live"; exit 1
fi

##Configuration specific things (some configs are built differently others only
##differ in run number of xml file)

TOPDIR=$SRCDIR-hmip
if [ ! -d "$TOPDIR" ]; then
  echo "Please set TOPDIR to the binary IPv6Suite directory. SRCDIR and BINDIR are different only for out of source builds" ; exit 1
fi
exit
EXDIRNAMES="HMIPv6Network"
SIMDIR=Examples/${EXDIRNAMES}
SIMRUN=4
SIMEXE=./${EXDIRNAMES}
INIFILE=HMIPv6Sait
XMLFILE=$INIFILE
NETNAME=`grep "network =" $SRCDIR/$SIMDIR/$INIFILE.ini |cut -d " " -f 3` #mipv6fastRANet
BEGINSIMTIME=7 #Used by graph-plot-graph.sh
SIMTIMELIMIT="549.95" #set to diff value by each config (limit in ini file to 2 decimal places.) Make sure ping will have times exceeding this as graph-omnetpp-runs.sh checks for this too as a proper run.

#confname really. used in output dir as well as modded ini file name and appended to modded xmlfile name
FILENAME=hmip-sait-noro
#Assumes output vector filename is omnetpp.vec if not change or make
#graphomnetpp-runs enforce this via modding ini file

echo topdir is $TOPDIR 
pushd $TOPDIR &>/dev/null
if [ $? -ne 0 ]; then
    echo "Failed to change to $TOPDIR"
    exit;
fi

#Assuming things made
pushd $SIMDIR &> /dev/null

cp -p $XMLFILE.xml{,.orig}
CONFIGURATIONS="hmip-sait-noro hmip-sait-nofast hmip-sait-mip hmip-sait-ro"
for conf in $CONFIGURATIONS
do
FILENAME=$conf
#Change Run number depending on conf
if [ "$conf" = "hmip-sait-ro" ]; then
echo hmip-sait-ro
  cp -p $XMLFILE.xml{.orig,}
  perl -i -pwe 's|routeOptimisation="off"|routeOptimisation="on"|g' $XMLFILE.xml
fi
if [ "$conf" = "hmip-sait-nofast" ]; then
echo hmip-sait-nofast
  cp -p $XMLFILE.xml{.orig,}
  convnofast $XMLFILE
  cp -p $XMLFILE{-nofast,}.xml
  rm $XMLFILE-nofast.xml
fi
if [ "$conf" = "hmip-sait-mip" ]; then
echo hmip-sait-mip
  cp -p $XMLFILE.xml{.orig,}
  perl -i -pwe 's|hierarchicalMIPv6Support="on"|hierarchicalMIPv6Support="off"|g' $XMLFILE.xml
fi

if [ ! -f $SCRIPTDIR/graph-omnetpp-runs.sh ]; then
    SCRIPTDIR=$SOURCEDIR/Etc/scripts
fi

. $SCRIPTDIR/graph-omnetpp-runs.sh
done
