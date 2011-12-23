#! /usr/bin/env Rscript

# full-duplex, two hosts

require('omnetpp')

dir = commandArgs(trailingOnly=TRUE)[1]

dataset <- loadDataset(paste(dir, '/*.sca',sep=''))
scalars <- dataset$scalars

duration <- scalars[scalars$name == 'duration',]$value
fullDuplex <- scalars[scalars$name == 'duplexMode',]$value
datarate <- scalars[scalars$name == 'datarate' & scalars$module != '.',]$value
packetLength <- scalars[scalars$name == 'packetLength',]$value
packetOverhead <- 38   # headers + preamble + SFD + IFG

expectedPacketRate <- datarate / (packetLength + packetOverhead) / 8

rcvdPackets <- scalars[scalars$name == 'rcvdPkBytes:count',]$value
rcvdPacketRate <- rcvdPackets / duration

if (all(fullDuplex==1)) {
    if (all(floor(expectedPacketRate) == floor(rcvdPacketRate))) {
        cat("PASS: ", expectedPacketRate[1], "packets/sec\n")
    } else {
        print(expectedPacketRate)
        print(rcvdPacketRate)
        cat("FAIL\n")
    }
} else if (all(fullDuplex==0)) {
    cat("ERROR: half duplex: TODO\n")
    #TODO
} else {
    print(fullDuplex)
    cat("ERROR: fullDuplex[] is messed up\n")
}

#cat( expectedPacketRate) 
#cat( rcvdPacketRate) 
