//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2010 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "ExampleQosClassifier.h"
#include "UserPriority.h"

#ifdef WITH_IPv4
#  include "inet/networklayer/ipv4/Ipv4Header_m.h"
#  include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif
#ifdef WITH_IPv6
#  include "inet/networklayer/ipv6/Ipv6Header.h"
#  include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif
#ifdef WITH_UDP
#  include "inet/transportlayer/udp/UdpHeader_m.h"
#endif
#ifdef WITH_TCP_COMMON
#  include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

namespace inet {

Define_Module(ExampleQosClassifier);

void ExampleQosClassifier::initialize()
{
    //TODO parameters
}

void ExampleQosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int ExampleQosClassifier::getUserPriority(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    int ipProtocol = -1;
    b ipHeaderLength = b(-1);

#ifdef WITH_IPv4
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv4) {
        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
        if (ipv4Header->getProtocolId() == IP_PROT_ICMP)
            return UP_BE; // ICMP class
        ipProtocol = ipv4Header->getProtocolId();
        ipHeaderLength = ipv4Header->getChunkLength();
    }
#endif

#ifdef WITH_IPv6
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv6) {
        const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
        if (ipv6Header->getProtocolId() == IP_PROT_IPv6_ICMP)
            return UP_BE; // ICMPv6 class
        ipProtocol = ipv6Header->getProtocolId();
        ipHeaderLength = ipv6Header->getChunkLength();
    }
#endif

    if (ipProtocol == -1)
        return UP_BE;

#ifdef WITH_UDP
    if (ipProtocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipHeaderLength);
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

#ifdef WITH_TCP_COMMON
    if (ipProtocol == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipHeaderLength);
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

} // namespace inet

