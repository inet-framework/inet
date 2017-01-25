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

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "ExampleQoSClassifier.h"
#include "UserPriority.h"

#ifdef WITH_IPv4
#undef WITH_IPv4  //FIXME //KLUDGE
#endif

#ifdef WITH_IPv4
#  include "inet/networklayer/ipv4/IPv4Header.h"
#  include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif
#ifdef WITH_IPv6
#  include "inet/networklayer/ipv6/IPv6Datagram.h"
#  include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif
#ifdef WITH_UDP
#  include "inet/transportlayer/udp/UDPPacket.h"
#endif
#ifdef WITH_TCP_COMMON
#  include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif

namespace inet {

Define_Module(ExampleQoSClassifier);

void ExampleQoSClassifier::initialize()
{
    //TODO parameters
}

void ExampleQoSClassifier::handleMessage(cMessage *msg)
{
    msg->ensureTag<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int ExampleQoSClassifier::getUserPriority(cMessage *msg)
{
    cPacket *ipData = nullptr;

#ifdef WITH_IPv4
    ipData = dynamic_cast<IPv4Header *>(msg);
    if (ipData && dynamic_cast<ICMPMessage *>(ipData->getEncapsulatedPacket()))
        return UP_BE; // ICMP class
#endif

#ifdef WITH_IPv6
    if (!ipData) {
        ipData = dynamic_cast<IPv6Datagram *>(msg);
        if (ipData && dynamic_cast<ICMPv6Message *>(ipData->getEncapsulatedPacket()))
            return UP_BE; // ICMPv6 class
    }
#endif

    if (!ipData)
        return UP_BE;

#ifdef WITH_UDP
    if (FlatPacket *udpPacket = dynamic_cast<FlatPacket*>(ipData->getEncapsulatedPacket())) {
        if (UDPHeader *udpHeader = dynamic_cast<UDPHeader *>(udpPacket->peekHeader())) {
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
    }
#endif

#ifdef WITH_TCP_COMMON
    tcp::TcpHeader *tcp = dynamic_cast<tcp::TcpHeader *>(ipData->getEncapsulatedPacket());
    if (tcp) {
        if (tcp->getDestPort() == 21 || tcp->getSrcPort() == 21)
            return UP_BK;
        if (tcp->getDestPort() == 80 || tcp->getSrcPort() == 80)
            return UP_BE;
        if (tcp->getDestPort() == 4000 || tcp->getSrcPort() == 4000)
            return UP_VI;
        if (tcp->getDestPort() == 5000 || tcp->getSrcPort() == 5000)
            return UP_VO;
        if (tcp->getDestinationPort() == 6000 || tcp->getSourcePort() == 6000) // not classified
            return -1;
    }
#endif

    return UP_BE;
}

} // namespace inet

