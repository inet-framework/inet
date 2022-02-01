//
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2010 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/common/ExampleQosClassifier.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"

#ifdef INET_WITH_ETHERNET
#  include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#endif
#ifdef INET_WITH_IPv4
#  include "inet/networklayer/ipv4/Ipv4Header_m.h"
#  include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif
#ifdef INET_WITH_IPv6
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif
#ifdef INET_WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif
#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif

namespace inet {

Define_Module(ExampleQosClassifier);

void ExampleQosClassifier::initialize()
{
    // TODO parameters
}

void ExampleQosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int ExampleQosClassifier::getUserPriority(cMessage *msg)
{
    int ipProtocol = -1;

#if defined(INET_WITH_ETHERNET) || defined(INET_WITH_IPv4) || defined(INET_WITH_IPv6) || defined(INET_WITH_UDP) || defined(INET_WITH_TCP_COMMON)
    auto packet = check_and_cast<Packet *>(msg);
    auto packetProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    int ethernetMacProtocol = -1;
    b ethernetMacHeaderLength = b(0);
    b ipHeaderLength = b(-1);
#endif

#ifdef INET_WITH_ETHERNET
    if (packetProtocol == &Protocol::ethernetMac) {
        const auto& ethernetMacHeader = packet->peekAtFront<EthernetMacHeader>();
        ethernetMacProtocol = ethernetMacHeader->getTypeOrLength();
        ethernetMacHeaderLength = ethernetMacHeader->getChunkLength();
    }
#endif

#ifdef INET_WITH_IPv4
    if (ethernetMacProtocol == ETHERTYPE_IPv4 || packetProtocol == &Protocol::ipv4) {
        const auto& ipv4Header = packet->peekDataAt<Ipv4Header>(ethernetMacHeaderLength);
        if (ipv4Header->getProtocolId() == IP_PROT_ICMP)
            return UP_BE; // ICMP class
        ipProtocol = ipv4Header->getProtocolId();
        ipHeaderLength = ipv4Header->getChunkLength();
    }
#endif

#ifdef INET_WITH_IPv6
    if (ethernetMacProtocol == ETHERTYPE_IPv6 || packetProtocol == &Protocol::ipv6) {
        const auto& ipv6Header = packet->peekDataAt<Ipv6Header>(ethernetMacHeaderLength);
        if (ipv6Header->getProtocolId() == IP_PROT_IPv6_ICMP)
            return UP_BE; // ICMPv6 class
        ipProtocol = ipv6Header->getProtocolId();
        ipHeaderLength = ipv6Header->getChunkLength();
    }
#endif

    if (ipProtocol == -1)
        return UP_BE;

#ifdef INET_WITH_UDP
    if (ipProtocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ethernetMacHeaderLength + ipHeaderLength);
        unsigned int srcPort = udpHeader->getSourcePort();
        unsigned int destPort = udpHeader->getDestinationPort();
        if (destPort == 21 || srcPort == 21)
            return UP_BK;
        if (destPort == 80 || srcPort == 80)
            return UP_BE;
        if (destPort == 4000 || srcPort == 4000)
            return UP_VI;
        if (destPort == 5000 || srcPort == 5000)
            return UP_VO;
        if (destPort == 6000 || srcPort == 6000) // not classified
            return -1;
    }
#endif

#ifdef INET_WITH_TCP_COMMON
    if (ipProtocol == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ethernetMacHeaderLength + ipHeaderLength);
        unsigned int srcPort = tcpHeader->getSourcePort();
        unsigned int destPort = tcpHeader->getDestinationPort();
        if (destPort == 21 || srcPort == 21)
            return UP_BK;
        if (destPort == 80 || srcPort == 80)
            return UP_BE;
        if (destPort == 4000 || srcPort == 4000)
            return UP_VI;
        if (destPort == 5000 || srcPort == 5000)
            return UP_VO;
        if (destPort == 6000 || srcPort == 6000) // not classified
            return -1;
    }
#endif

    return UP_BE;
}

void ExampleQosClassifier::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (!strcmp("in", g->getName()))
        registerService(protocol, gate("out"), servicePrimitive);
    else if (!strcmp("out", g->getName()))
        registerService(protocol, gate("in"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

void ExampleQosClassifier::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("in", g->getName()))
        registerProtocol(protocol, gate("out"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

} // namespace inet

