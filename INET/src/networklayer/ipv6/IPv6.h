//
// Copyright (C) 2005 Andras Varga
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


#ifndef __IPv6_H__
#define __IPv6_H__

#include <omnetpp.h>
#include "QueueBase.h"
#include "RoutingTable6.h"
#include "ICMPv6.h"
#include "IPv6NeighbourDiscovery.h"
#include "IPv6Datagram.h"
#include "IPv6FragBuf.h"
#include "ProtocolMap.h"

class ICMPv6Message;

/**
 * IPv6 implementation.
 */
class INET_API IPv6 : public QueueBase
{
  protected:
    IInterfaceTable *ift;
    RoutingTable6 *rt;
    IPv6NeighbourDiscovery *nd;
    ICMPv6 *icmp;

    // working vars
    long curFragmentId; // counter, used to assign unique fragmentIds to datagrams
    IPv6FragBuf fragbuf;  // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    ProtocolMapping mapping; // where to send packets after decapsulation

    // statistics
    int numMulticast;
    int numLocalDeliver;
    int numDropped;
    int numUnroutable;
    int numForwarded;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual InterfaceEntry *getSourceInterfaceFrom(cPacket *msg);

    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    /**
     * Encapsulate packet coming from higher layers into IPv6Datagram
     */
    virtual IPv6Datagram *encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE);

    /**
     * Handle IPv6Datagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handleDatagramFromNetwork(IPv6Datagram *datagram);

    /**
     * Handle messages (typically packets to be send in IPv6) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(cPacket *msg);

    /**
     * Handle incoming ICMP messages.
     */
    virtual void handleReceivedICMP(ICMPv6Message *msg);

    /**
     * Fragment packet if needed, then send it. The optional output gate
     * index is only used if higher layer protocol explicitly requests
     * the datagram to be sent out on a specific interface, bypassing
     * the routing table.
     */
    virtual void fragmentAndRoute(IPv6Datagram *datagram, InterfaceEntry *destIE=NULL);

    /**
     * Performs routing. Based on the routing decision, it dispatches to
     * isLocalAddress() for local packets, to fragmentAndSend() for forwarded packets,
     * to routeMulticastPacket() for multicast packets, or drops the packet if
     * it's unroutable or forwarding is off.
     */
    virtual void routePacket(IPv6Datagram *datagram, InterfaceEntry *destIE, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void routeMulticastPacket(IPv6Datagram *datagram, InterfaceEntry *destIE, InterfaceEntry *fromIE);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void isLocalAddress(IPv6Datagram *datagram);

    /**
     * Decapsulate and return encapsulated packet after attaching IPv6ControlInfo.
     */
    virtual cPacket *decapsulate(IPv6Datagram *datagram);

    /**
     * Last hoplimit check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(IPv6Datagram *datagram, InterfaceEntry *ie, const MACAddress& macAddr);

  public:
    IPv6() {}

  protected:
    /**
     * Initialization
     */
    virtual void initialize();

    /**
     * Processing of IPv6 datagrams. Called when a datagram reaches the front
     * of the queue.
     */
    virtual void endService(cPacket *msg);
};


#endif


