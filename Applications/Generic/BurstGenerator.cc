/*
    file: BurstGenerator.cc
    Purpose: Traffic Generator that sends out a number of
        packets all at once, then stops
    author: Jochen Reber
*/

#include <omnetpp.h>
#include "BurstGenerator.h"
#include "IPControlInfo_m.h"
#include "IPDatagram.h"
#include "ICMP.h"

Define_Module_Like(BurstGenerator, GeneratorAppOut);

void BurstGenerator::initialize()
{
    burstSize = par("burstPackets");
    packetSize = par("generationSize");
    usesTCPProt = par("tcpProtocol");
    destAddress = par("destAddress").stringValue();
}

void BurstGenerator::activity()
{
    int packetsSent;
    int contCtr = id()*10000+100;

    wait(1);

    for (packetsSent = 0; packetsSent < burstSize; packetsSent++)
    {
        cPacket *transportPacket = new cPacket("burstgen-pk");
        transportPacket->setLength(packetSize);
        transportPacket->addPar("content") = contCtr++;

        IPControlInfo *controlInfo = new IPControlInfo();
        controlInfo->setDestAddr(destAddress);
        controlInfo->setProtocol(usesTCPProt ? IP_PROT_TCP : IP_PROT_UDP);
        transportPacket->setControlInfo(controlInfo);

        send(transportPacket, "out");
    }

    if (burstSize > 0)
    {
        ev << "Burst Generator: " << burstSize << " packets sent to "<< destAddress << "\n";
    }

}

