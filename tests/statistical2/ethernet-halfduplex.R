#! /usr/bin/env Rscript

# full-duplex, two hosts

require('omnetpp')

dir = commandArgs(trailingOnly=TRUE)[1]

dataset <- loadDataset(paste(dir, '/*.sca',sep=''))
#dataset <- loadDataset('/home/andras/inet/tests/statistical2/res1/*.sca')
scalars <- dataset$scalars

print(as.character(dataset$runattrs[dataset$runattrs$name == 'iterationvars',]$value))

duration <- scalars[scalars$name == 'duration',]$value
datarate <- scalars[scalars$name == 'datarate' & scalars$module != '.',]$value
numHosts <- scalars[scalars$name == 'numHosts' & scalars$module == '.',]$value
fullDuplex <- scalars[scalars$name == 'duplexMode',]$value
payloadLength <- scalars[scalars$name == 'packetLength',]$value

minPacketLength = ifelse(datarate[1] == 1e9, 512, 64)  # payload + headers (excluding preamble, SFD, IFG)
headerLength <- 18   # headers (excluding preamble, SFD, IFG)

packetLength <- max(payloadLength + headerLength, minPacketLength)
transmitOverhead <- 20  # preamble + SFD + IFG

expectedPacketRate <- datarate / (packetLength + transmitOverhead) / 8

rcvdPackets <- scalars[scalars$name == 'rcvdPkBytes:count',]$value
rcvdPacketRate <- rcvdPackets / duration

#print(rcvdPackets)

names <- c(`$numHosts=64, $datarate=10Mbps, $packetLength=1500B` = 0.98,
           `$numHosts=64, $datarate=10Mbps, $packetLength=1500B` = 0.99
           )

#print(scalars[scalars$name == 'datarate' & scalars$module != '.',])

#print(numHosts)
#print(datarate)
#print(expectedPacketRate)
#print(sum(rcvdPacketRate))
print(sum(rcvdPacketRate) / expectedPacketRate)


if (all(fullDuplex==0)) {
    cat("ERROR: half duplex: TODO\n")
    
} else {
    print(fullDuplex)
    cat("ERROR: fullDuplex[] is messed up\n")
}

#cat( expectedPacketRate) 
#cat( rcvdPacketRate) 
