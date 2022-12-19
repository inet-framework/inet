//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/diffserv/DscpMarker.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"
#include "inet/networklayer/diffserv/Dscp_m.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#endif // #ifdef INET_WITH_ETHERNET

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef INET_WITH_IPv6

namespace inet {

using namespace DiffservUtil;

Define_Module(DscpMarker);

simsignal_t DscpMarker::packetMarkedSignal = registerSignal("packetMarked");

void DscpMarker::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        parseDSCPs(par("dscps"), "dscps", dscps);
        if (dscps.empty())
            dscps.push_back(DSCP_BE);
        while ((int)dscps.size() < gateSize("in"))
            dscps.push_back(dscps.back());

        numRcvd = 0;
        numMarked = 0;
        WATCH(numRcvd);
        WATCH(numMarked);
    }
}

void DscpMarker::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void DscpMarker::pushPacket(Packet *packet, cGate *inputGate)
{
    Enter_Method("pushPacket");
    take(packet);
    numRcvd++;
    int dscp = dscps.at(inputGate->getIndex());
    if (markPacket(packet, dscp)) {
        emit(packetMarkedSignal, packet);
        numMarked++;
    }
    pushOrSendPacket(packet, outputGate, consumer);
}

void DscpMarker::refreshDisplay() const
{
    char buf[50] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd: %d ", numRcvd);
    if (numMarked > 0)
        sprintf(buf + strlen(buf), "mark:%d ", numMarked);
    getDisplayString().setTagArg("t", 0, buf);
}

bool DscpMarker::markPacket(Packet *packet, int dscp)
{
    EV_DETAIL << "Marking packet with dscp=" << dscpToString(dscp) << "\n";

    b offset(0);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();

    if (protocol->getLayer() == Protocol::LinkLayer) {
        if (protocol == &Protocol::ethernetMac) {
#ifdef INET_WITH_ETHERNET
            auto ethHeader = packet->peekDataAt<EthernetMacHeader>(offset);
            if (isEth2Header(*ethHeader)) {
                offset += ethHeader->getChunkLength();
                protocol = ProtocolGroup::getEthertypeProtocolGroup()->getProtocol(ethHeader->getTypeOrLength());
            }
#else
        throw cRuntimeError("Ethernet feature is disabled");
#endif // #ifdef INET_WITH_ETHERNET
        }
        else
            return false;
    }

    if (protocol->getLayer() != Protocol::NetworkLayer)
        return false;
#ifdef INET_WITH_IPv4
    else if (protocol == &Protocol::ipv4) {
        packet->removeTagIfPresent<NetworkProtocolInd>();
        auto ipv4Header = packet->removeDataAt<Ipv4Header>(offset);
        ipv4Header->setDscp(dscp);
        ipv4Header->updateCrc(); // recalculate IP header checksum
        auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
        networkProtocolInd->setProtocol(protocol);
        networkProtocolInd->setNetworkProtocolHeader(ipv4Header);
        packet->insertDataAt(ipv4Header, offset);
        return true;
    }
#endif // ifdef INET_WITH_IPv4
#ifdef INET_WITH_IPv6
    else if (protocol == &Protocol::ipv6) {
        packet->removeTagIfPresent<NetworkProtocolInd>();
        auto ipv6Header = packet->removeDataAt<Ipv6Header>(offset);
        ipv6Header->setDscp(dscp);
        auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
        networkProtocolInd->setProtocol(protocol);
        networkProtocolInd->setNetworkProtocolHeader(ipv6Header);
        packet->insertDataAt(ipv6Header, offset);
        return true;
    }
#endif // ifdef INET_WITH_IPv6
    return false;
}

} // namespace inet

