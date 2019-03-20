#!/bin/bash

# parse command line arguments
# -f --filename        csv file to be created. Existing file will be overwritten.

# parsing command line arguments from: http://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -f|--filename)
    CSVFILE="$2"
    shift # past argument
    ;;
    *)
          # unknown option
    ;;
esac
shift # past argument or value
done

if [ -z "$CSVFILE" ]
then
   echo "File name mandatory. Usage: ./parse_omnetpp_logs.sh -f <filename> Note: File name given will be overwritten."
   exit
fi

touch ${CSVFILE}
echo "numNodes;networkSize;sigma;pathLossModel;useADR;adrType;repetition;DER;energyConsumed;totalSentPackets;nsReceivedPackets;LostPackets" > ${CSVFILE}

# only omnetpp scalar files assumed to be in results directory
dir=results/*

# loop through all files in results directory
for file in $dir
do
    pathloss=$(grep "pathLossType" $file | awk '{print $3}' |  tr -d '\\"')
    der=$(grep "LoRa_NS_DER" $file | awk '{print $4}')
    sentPackets=$(grep "sentPackets" $file | awk '{sum+=$4} END {print sum}')
    nsReceivedPackets=$(grep "numOfReceivedPackets" $file | awk '{print $4}')
    lostPackets="$((sentPackets - nsReceivedPackets))"
    line=$(grep "iterationvars2" $file )
    energyConsumed=$(grep "totalEnergyConsumed" $file | awk '{sum+=sprintf("%f",$4)}END{printf "%.6f\n",sum}')

    # match ADR/NoADR from file name
    fileregex="([A-Za-z]+)-([A-Za-z]+)-([0-9]+).sca" 
    if [[ $file =~ $fileregex ]]
    then
       adrText=${BASH_REMATCH[2]}
       adrTrue="ADR"
       adrFalse="NoADR"
       if [ "$adrText" == "$adrTrue" ]
       then
          adrEnabled="True"
       fi 
       if [ "$adrText" == "$adrFalse" ]
       then
          adrEnabled="False"
          adrMethod="NA"
       fi 
    fi
    
    # uncomment for old logs, matches attr iterationvars2 "$sigma=0, $networkSize=320m, $ADR=true, $adrMethod=\"avg\", $numberOfNodes=400, $repetition=9"
    #exp='\$sigma=([-+]?[0-9]*\.?[0-9]+), \$networkSize=([0-9]+)m, \$ADR=(.*), \$adrMethod=\\"(.*)\\", \$numberOfNodes=([0-9]+), \$repetition=([0-9]+)'

    # match parameters for simulation run
    # new logs, matches: attr iterationvars2 "$sigma=0, $networkSize=320m, $adrMethod=\"avg\", $numberOfNodes=100, $repetition=0"
    exp='\$sigma=([-+]?[0-9]*\.?[0-9]+), \$networkSize=([0-9]+)m, \$adrMethod=\\"(.*)\\", \$numberOfNodes=([0-9]+), \$repetition=([0-9]+)'
    if [[ $line =~ $exp ]]
    then
       if [ "$adrEnabled" == "True" ]
       then
          adrMethod="${BASH_REMATCH[3]}"
       fi
       # uncomment for old logs
       #echo ${BASH_REMATCH[5]}";"${BASH_REMATCH[2]}";"${BASH_REMATCH[1]}";"${pathloss}";"${BASH_REMATCH[3]}";"${BASH_REMATCH[4]}";"${BASH_REMATCH[6]}";"$der";"$energyConsumed";"$sentPackets";"$nsReceivedPackets";"$lostPackets >> ${CSVFILE}
       echo ${BASH_REMATCH[4]}";"${BASH_REMATCH[2]}";"${BASH_REMATCH[1]}";"${pathloss}";"${adrEnabled}";"${adrMethod}";"${BASH_REMATCH[5]}";"$der";"$energyConsumed";"$sentPackets";"$nsReceivedPackets";"$lostPackets >> ${CSVFILE}
    else
       echo $file", bash regex not matched"
    fi
done
