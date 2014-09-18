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

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211eClassifier.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#ifdef WITH_IPv4
  #include "inet/networklayer/ipv4/IPv4Datagram.h"
  #include "inet/networklayer/ipv4/ICMPMessage_m.h"
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
  #include "inet/networklayer/ipv6/IPv6Datagram.h"
  #include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#endif // ifdef WITH_IPv6
#ifdef WITH_UDP
  #include "inet/transportlayer/udp/UDPPacket_m.h"
#endif // ifdef WITH_UDP
#ifdef WITH_TCP_COMMON
  #include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif // ifdef WITH_TCP_COMMON

namespace inet {

namespace ieee80211 {

Register_Class(Ieee80211eClassifier);

Ieee80211eClassifier::Ieee80211eClassifier()
{
    defaultAC = 0;
    defaultManagement = 3;
}

int Ieee80211eClassifier::getNumQueues()
{
    return 4;
}

int Ieee80211eClassifier::classifyPacket(cMessage *frame)
{
    ASSERT(check_and_cast<Ieee80211DataOrMgmtFrame *>(frame));
    cPacket *ipData = NULL;    // must be initialized in case neither IPv4 nor IPv6 is present

    // if this is a management type, use a pre-configured default class
    if (dynamic_cast<Ieee80211ManagementFrame *>(frame))
        return defaultManagement;

    // we have a data packet
    cPacket *encapsulatedNetworkPacket = PK(frame)->getEncapsulatedPacket();
    ASSERT(encapsulatedNetworkPacket);    // frame must contain an encapsulated network data frame

#ifdef WITH_IPv4
    ipData = dynamic_cast<IPv4Datagram *>(encapsulatedNetworkPacket);
    if (ipData && dynamic_cast<ICMPMessage *>(ipData->getEncapsulatedPacket()))
        return 1; // ICMP class
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
    if (!ipData) {
        ipData = dynamic_cast<IPv6Datagram *>(encapsulatedNetworkPacket);
        if (ipData && dynamic_cast<ICMPv6Message *>(ipData->getEncapsulatedPacket()))
            return 1; // ICMPv6 class
    }
#endif // ifdef WITH_IPv6

    if (!ipData)
        return defaultAC; // neither IPv4 nor IPv6 packet (unknown protocol) = default AC

#ifdef WITH_UDP
    UDPPacket *udp = dynamic_cast<UDPPacket *>(ipData->getEncapsulatedPacket());
    if (udp) {
        if (udp->getDestinationPort() == 21 || udp->getSourcePort() == 21)
            return 0;
        if (udp->getDestinationPort() == 80 || udp->getSourcePort() == 80)
            return 1;
        if (udp->getDestinationPort() == 4000 || udp->getSourcePort() == 4000)
            return 2;
        if (udp->getDestinationPort() == 5000 || udp->getSourcePort() == 5000)
            return 3;
    }
#endif // ifdef WITH_UDP

#ifdef WITH_TCP_COMMON
    tcp::TCPSegment *tcp = dynamic_cast<tcp::TCPSegment *>(ipData->getEncapsulatedPacket());
    if (tcp) {
        if (tcp->getDestPort() == 21 || tcp->getSrcPort() == 21)
            return 0;
        if (tcp->getDestPort() == 80 || tcp->getSrcPort() == 80)
            return 1;
        if (tcp->getDestPort() == 4000 || tcp->getSrcPort() == 4000)
            return 2;
        if (tcp->getDestPort() == 5000 || tcp->getSrcPort() == 5000)
            return 3;
    }
#endif // ifdef WITH_TCP_COMMON

    return defaultAC;
}

} // namespace ieee80211

} // namespace inet

