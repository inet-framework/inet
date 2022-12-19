//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/marker/EcnMarker.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/networklayer/common/L3Tools.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#endif // #ifdef INET_WITH_ETHERNET

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef INET_WITH_IPv4

namespace inet {
namespace queueing {

Define_Module(EcnMarker);

void EcnMarker::markPacket(Packet *packet)
{
    const auto& ecnReq = packet->findTag<EcnReq>();
    if (ecnReq != nullptr) {
        EV_DETAIL << "Marking packet with ECN" << EV_ENDL;
        setEcn(packet, static_cast<IpEcnCode>(ecnReq->getExplicitCongestionNotification()));
    }
}

void EcnMarker::setEcn(Packet *packet, IpEcnCode ecn)
{
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
    }

    if (protocol == &Protocol::ipv4) {
#ifdef INET_WITH_IPv4
        packet->removeTagIfPresent<NetworkProtocolInd>();
        auto ipv4Header = packet->removeDataAt<Ipv4Header>(offset);
        ipv4Header->setEcn(ecn);
        ipv4Header->updateCrc(); // recalculate IP header checksum
        auto networkProtocolInd = packet->addTagIfAbsent<NetworkProtocolInd>();
        networkProtocolInd->setProtocol(protocol);
        networkProtocolInd->setNetworkProtocolHeader(ipv4Header);
        packet->insertDataAt(ipv4Header, offset);
#else
        throw cRuntimeError("IPv4 feature is disabled");
#endif
    }
}

IpEcnCode EcnMarker::getEcn(const Packet *packet)
{
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
    }
    if (protocol == &Protocol::ipv4) {
#ifdef INET_WITH_IPv4
        auto ipv4Header = packet->peekDataAt<Ipv4Header>(offset);
        return static_cast<IpEcnCode>(ipv4Header->getEcn());
#endif
    }
    return IP_ECN_NOT_ECT;
}

} // namespace queueing
} // namespace inet

