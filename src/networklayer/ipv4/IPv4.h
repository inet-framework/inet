//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_IP_H
#define __INET_IP_H

#include "QueueBase.h"
#include "InterfaceTableAccess.h"
#include "RoutingTableAccess.h"
#include "IRoutingTable.h"
#include "ICMPAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv4FragBuf.h"
#include "ProtocolMap.h"

#ifdef WITH_MANET
#include "ControlManetRouting_m.h"
#endif


class ARPPacket;
class ICMPMessage;

// ICMP type 2, code 4: fragmentation needed, but don't-fragment bit set
const int ICMP_FRAGMENTATION_ERROR_CODE = 4;


/**
 * Implements the IPv4 protocol.
 */
class INET_API IPv4 : public QueueBase
{
  protected:
    IRoutingTable *rt;
    IInterfaceTable *ift;
    ICMPAccess icmpAccess;
    cGate *queueOutGate; // the most frequently used output gate
    bool manetRouting;

    // config
    int defaultTimeToLive;
    int defaultMCTimeToLive;
    simtime_t fragmentTimeoutTime;
    bool forceBroadcast;

    // working vars
    long curFragmentId; // counter, used to assign unique fragmentIds to datagrams
    IPv4FragBuf fragbuf;  // fragmentation reassembly buffer
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
     * Encapsulate packet coming from higher layers into IPv4Datagram, using
     * the control info attached to the packet.
     */
    virtual IPv4Datagram *encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE);

    /**
     * Encapsulate packet coming from higher layers into IPv4Datagram, using
     * the given control info. Override if you subclassed controlInfo and/or
     * want to add options etc to the datagram.
     */
    virtual IPv4Datagram *encapsulate(cPacket *transportPacket, InterfaceEntry *&destIE, IPv4ControlInfo *controlInfo);

    /**
     * Creates a blank IPv4 datagram. Override when subclassing IPv4Datagram is needed
     */
    virtual IPv4Datagram *createIPv4Datagram(const char *name);

    /**
     * Handle IPv4Datagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handlePacketFromNetwork(IPv4Datagram *datagram);

    /**
     * Handle messages (typically packets to be send in IPv4) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(cPacket *msg);

    /**
     * Handle incoming ARP packets by sending them over "queueOut" to ARP.
     */
    virtual void handleARP(ARPPacket *msg);

    /**
     * Handle incoming ICMP messages.
     */
    virtual void handleReceivedICMP(ICMPMessage *msg);

    /**
     * Performs routing. Based on the routing decision, it dispatches to
     * reassembleAndDeliver() for local packets, to fragmentAndSend() for forwarded packets,
     * to handleMulticastPacket() for multicast packets, or drops the packet if
     * it's unroutable or forwarding is off.
     */
    virtual void routePacket(IPv4Datagram *datagram, InterfaceEntry *destIE, bool fromHL,IPv4Address* nextHopAddrPtr);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void routeMulticastPacket(IPv4Datagram *datagram, InterfaceEntry *destIE, InterfaceEntry *fromIE);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void reassembleAndDeliver(IPv4Datagram *datagram);

    /**
     * Decapsulate and return encapsulated packet after attaching IPv4ControlInfo.
     */
    virtual cPacket *decapsulateIP(IPv4Datagram *datagram);

    /**
     * Fragment packet if needed, then send it to the selected interface using
     * sendDatagramToOutput().
     */
    virtual void fragmentAndSend(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr);

    /**
     * Last TTL check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr);

#ifdef WITH_MANET
    virtual void controlMessageToManetRouting(int,IPv4Datagram *datagram);
#endif
    virtual void dsrFillDestIE(IPv4Datagram *, InterfaceEntry *&destIE,IPv4Address &nextHopAddress);

    const IPv4RouteRule * checkInputRule(const IPv4Datagram*);
    const IPv4RouteRule * checkOutputRule(const IPv4Datagram*,const InterfaceEntry*);
    const IPv4RouteRule * checkOutputRuleMulticast(const IPv4Datagram*);

  public:
    IPv4() {}

  protected:
    /**
     * Initialization
     */
    virtual void initialize();

    /**
     * Processing of IPv4 datagrams. Called when a datagram reaches the front
     * of the queue.
     */
    virtual void endService(cPacket *msg);
};

#endif

