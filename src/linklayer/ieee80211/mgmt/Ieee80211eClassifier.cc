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


#include <omnetpp.h>
#include "Ieee80211eClassifier.h"
#include "IPDatagram.h"
#include "ICMPMessage.h"
#include "IPv6Datagram.h"
#include "ICMPMessage.h"
#include "IPv6NDMessage_m.h"
#include "UDPPacket_m.h"
#include "TCPSegment.h"

Register_Class(Ieee80211eClassifier);

#define DEFAULT 0

Ieee80211eClassifier::Ieee80211eClassifier()
{
    defaultAC=DEFAULT;
}


int Ieee80211eClassifier::getNumQueues()
{
    return 4;
}

int Ieee80211eClassifier::classifyPacket(cMessage *msg)
{

    IPDatagram *ipv4data = dynamic_cast<IPDatagram *>(PK(msg)->getEncapsulatedPacket());
    IPv6Datagram *ipv6data = dynamic_cast<IPv6Datagram *>(PK(msg)->getEncapsulatedPacket());

    if (ipv4data==NULL && ipv6data==NULL)
    {
        return 3;
    }
	UDPPacket *udp=NULL;
    TCPSegment *tcp=NULL;
    ICMPMessage *icmp=NULL;
    ICMPv6Message *icmpv6=NULL;
    if (ipv4data)
    {
        icmp = dynamic_cast<ICMPMessage *>(ipv4data->getEncapsulatedPacket());
    	udp=dynamic_cast<UDPPacket *>(ipv4data->getEncapsulatedPacket());
        tcp=dynamic_cast<TCPSegment *>(ipv4data->getEncapsulatedPacket());
    }
    if (ipv6data)
    {
        icmpv6 = dynamic_cast<ICMPv6Message *>(ipv6data->getEncapsulatedPacket());
    	udp=dynamic_cast<UDPPacket *>(ipv6data->getEncapsulatedPacket());
        tcp=dynamic_cast<TCPSegment *>(ipv6data->getEncapsulatedPacket());

    }
    if (icmp || icmpv6)
        return 1;

    if (udp)
    {
        if (udp->getDestinationPort() == 5000 || udp->getSourcePort() == 5000)  //voice
    	   return 3;
        if (udp->getDestinationPort() == 4000 || udp->getSourcePort() == 4000)  //video
           return 2;
        if (udp->getDestinationPort() == 80 || udp->getSourcePort() == 80)  //voice
            return 1;
        if (udp->getDestinationPort() == 21 || udp->getSourcePort() == 21)  //voice
            return 0;
    }
    if (tcp)
    {
        if (tcp->getDestPort() == 80 || tcp->getSrcPort() == 80)
            return 1;
        if (tcp->getDestPort() == 21 || tcp->getSrcPort() == 21)
            return 0;
        if (tcp->getDestPort() == 4000 || tcp->getSrcPort() == 4000)
            return 2;
        if (tcp->getDestPort() == 5000 || tcp->getSrcPort() == 5000)
            return 3;
    }
   	return defaultAC;
}


