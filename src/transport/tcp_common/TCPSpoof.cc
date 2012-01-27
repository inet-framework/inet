//
// Copyright 2006 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPSpoof.h"

#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"


Define_Module(TCPSpoof);

simsignal_t TCPSpoof::sentPkSignal = SIMSIGNAL_NULL;

void TCPSpoof::initialize()
{
    sentPkSignal = registerSignal("sentPk");
    simtime_t t = par("t").doubleValue();
    scheduleAt(t, new cMessage("timer"));
}

void TCPSpoof::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        error("Does not process incoming messages");

    sendSpoofPacket();
    delete msg;
}

void TCPSpoof::sendSpoofPacket()
{
    TCPSegment *tcpseg = new TCPSegment("spoof");

    IPvXAddress srcAddr = IPvXAddressResolver().resolve(par("srcAddress"));
    IPvXAddress destAddr = IPvXAddressResolver().resolve(par("destAddress"));
    int srcPort = par("srcPort");
    int destPort = par("destPort");
    bool isSYN = par("isSYN");
    unsigned long seq = par("seqNo").longValue()==-1 ? chooseInitialSeqNum() : par("seqNo").longValue();

    // one can customize the following according to concrete needs
    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);
    tcpseg->setByteLength(TCP_HEADER_OCTETS);
    tcpseg->setSequenceNo(seq);
    //tcpseg->setAckNo(...);
    tcpseg->setSynBit(isSYN);
    tcpseg->setWindow(16384);

    sendToIP(tcpseg, srcAddr, destAddr);
}

void TCPSpoof::sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest)
{
    EV << "Sending: ";
    //printSegmentBrief(tcpseg);

    if (!dest.isIPv6())
    {
        // send over IPv4
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get4());
        controlInfo->setDestAddr(dest.get4());
        tcpseg->setControlInfo(controlInfo);

        emit(sentPkSignal, tcpseg);
        send(tcpseg, "ipv4Out");
    }
    else
    {
        // send over IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get6());
        controlInfo->setDestAddr(dest.get6());
        tcpseg->setControlInfo(controlInfo);

        emit(sentPkSignal, tcpseg);
        send(tcpseg, "ipv6Out");
    }
}

unsigned long TCPSpoof::chooseInitialSeqNum()
{
    // choose an initial send sequence number in the same way as TCP does
    return (unsigned long)SIMTIME_DBL(fmod(simTime()*250000.0, 1.0+(double)(unsigned)0xffffffffUL)) & 0xffffffffUL;
}


