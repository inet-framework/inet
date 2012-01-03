#! /usr/bin/env Rscript

#
# Verify data rates for half-duplex ethernet, two hosts
#
# References:
#
# "Ethernet: Distributed Packet Switching for Local Computer Networks",
# Robert M. Metcalfe and David R. Boggs, CSL·75·7 May 1975.
# http://ethernethistory.typepad.com/papers/EthernetPaper.pdf
#
# "Measured Performance of an Ethernet Local Network",
# John F. Shoch and Jon A. Hupp, February 1980, Xerox Corporation 1980.
# http://ethernethistory.typepad.com/papers/EthernetPerformance.pdf
#
# "Performance  Characteristics of 2 Ethernets: an Experimental Study"
# Timothy A. Gonsalves. Technical Report: CSL-TR-87-317. March 1987.
# http://www.cs.utsa.edu/faculty/boppana/courses/cnet-common/eth-gonsalves-sigmetrics85.pdf
#
# "Performance Evaluation of Different Ethernet LANs Connected by Switches and Hubs",
# Ikram Ud Din, Saeed Mahfooz, Muhammad Adnan. European Journal of Scientific Research,
# ISSN 1450-216X Vol.37 No.3 (2009), pp.461-470.
# http://www.eurojournals.com/ejsr_37_3_11.pdf
#

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
    cat("ERROR: this script expects all MACs to be configured as half duplex\n")
    print(fullDuplex)

#cat( expectedPacketRate)
#cat( rcvdPacketRate)
