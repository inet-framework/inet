#! /usr/bin/env Rscript

#
# Verify data rates for full-duplex ethernet, two hosts.
#
# References:
#
# "Maximum Packet Rates for Full-Duplex Ethernet", Scott Karlin and
# Larry Peterson. Technical Report TR–645–02. February 14, 2002.
#
# "Gigabit Ethernet - TCP/IP: Maximum Ethernet frames and data throughput
# rate calculations." Network Security Toolkit (NST) Wiki, NST-2011.
# http://wiki.networksecuritytoolkit.org/nstwiki/index.php/File:Gigabit_ethernet_tcpip.png
#

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
} else {
    cat("ERROR: this script expects all MACs to be configured as full duplex\n")
    print(fullDuplex)
}

#cat( expectedPacketRate)
#cat( rcvdPacketRate)
