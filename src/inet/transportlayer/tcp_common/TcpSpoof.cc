//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_common/TcpSpoof.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"

namespace inet {
namespace tcp {

Define_Module(TcpSpoof);

void TcpSpoof::initialize()
{
    simtime_t t = par("t");
    scheduleAt(t, new cMessage("timer"));
}

void TcpSpoof::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
        throw cRuntimeError("Does not process incoming messages");

    sendSpoofPacket();
    delete msg;
}

void TcpSpoof::sendSpoofPacket()
{
    Packet *packet = new Packet("spoof");
    const auto& tcpseg = makeShared<TcpHeader>();

    L3Address srcAddr = L3AddressResolver().resolve(par("srcAddress"));
    L3Address destAddr = L3AddressResolver().resolve(par("destAddress"));
    int srcPort = par("srcPort");
    int destPort = par("destPort");
    bool isSYN = par("isSYN");
    unsigned long seq = par("seqNo").intValue() == -1 ? chooseInitialSeqNum() : par("seqNo");

    // one can customize the following according to concrete needs
    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);
    tcpseg->setChunkLength(TCP_MIN_HEADER_LENGTH);
    tcpseg->setSequenceNo(seq);
//    tcpseg->setAckNo(...);
    tcpseg->setSynBit(isSYN);
    tcpseg->setWindow(16384);
    // a receiver checks the checksum by the mode of the header; as by default in Tcp, the
    // checksum is declared correct
    tcpseg->setChecksumMode(CHECKSUM_DECLARED_CORRECT);
    insertTransportProtocolHeader(packet, Protocol::tcp, tcpseg);

    sendToIP(packet, srcAddr, destAddr);
}

void TcpSpoof::sendToIP(Packet *pk, L3Address src, L3Address dest)
{
    EV_INFO << "Sending: ";
//    printSegmentBrief(tcpseg);

    ASSERT(pk != nullptr);
    const IL3AddressType *addressType = dest.getAddressType();
    pk->addTagIfAbsent<TransportProtocolInd>()->setProtocol(&Protocol::tcp);
    pk->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    auto addresses = pk->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(src);
    addresses->setDestAddress(dest);

    emit(packetSentSignal, pk);
    send(pk, "ipOut");
}

unsigned long TcpSpoof::chooseInitialSeqNum()
{
    // the ISS of the clock as TcpConnection::selectInitialSeqNum() takes it: one tick in 4
    // microseconds, modulo 2^32 (RFC 9293 section 3.4.1); the time is a double before the
    // multiplication, because a simtime_t cannot hold the modulus
    return (unsigned long)fmod(SIMTIME_DBL(simTime()) * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL) & 0xffffffffUL;
}

} // namespace tcp
} // namespace inet

