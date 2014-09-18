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

#include "inet/transportlayer/tcp_common/TCPSpoof.h"

#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {

namespace tcp {

Define_Module(TCPSpoof);

simsignal_t TCPSpoof::sentPkSignal = registerSignal("sentPk");

void TCPSpoof::initialize()
{
    simtime_t t = par("t").doubleValue();
    scheduleAt(t, new cMessage("timer"));
}

void TCPSpoof::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("Does not process incoming messages");

    sendSpoofPacket();
    delete msg;
}

void TCPSpoof::sendSpoofPacket()
{
    TCPSegment *tcpseg = new TCPSegment("spoof");

    L3Address srcAddr = L3AddressResolver().resolve(par("srcAddress"));
    L3Address destAddr = L3AddressResolver().resolve(par("destAddress"));
    int srcPort = par("srcPort");
    int destPort = par("destPort");
    bool isSYN = par("isSYN");
    unsigned long seq = par("seqNo").longValue() == -1 ? chooseInitialSeqNum() : par("seqNo").longValue();

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

void TCPSpoof::sendToIP(TCPSegment *tcpseg, L3Address src, L3Address dest)
{
    EV_INFO << "Sending: ";
    //printSegmentBrief(tcpseg);

    IL3AddressType *addressType = dest.getAddressType();
    INetworkProtocolControlInfo *controlInfo = addressType->createNetworkProtocolControlInfo();
    controlInfo->setTransportProtocol(IP_PROT_TCP);
    controlInfo->setSourceAddress(src);
    controlInfo->setDestinationAddress(dest);
    tcpseg->setControlInfo(check_and_cast<cObject *>(controlInfo));

    emit(sentPkSignal, tcpseg);
    send(tcpseg, "ipOut");
}

unsigned long TCPSpoof::chooseInitialSeqNum()
{
    // choose an initial send sequence number in the same way as TCP does
    return (unsigned long)SIMTIME_DBL(fmod(simTime() * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL)) & 0xffffffffUL;
}

} // namespace tcp

} // namespace inet

