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

#ifndef __INET_IPv4_H
#define __INET_IPv4_H

#include "INETDefs.h"

#include "ICMPAccess.h"
#include "IPv4FragBuf.h"
#include "ProtocolMap.h"
#include "QueueBase.h"

#ifdef WITH_MANET
#include "ControlManetRouting_m.h"
#endif

#include "IPv4Datagram.h"
#include "ILifecycle.h"


class ARPPacket;
class ICMPMessage;
class IInterfaceTable;
class IPv4Datagram;
class IIPv4RoutingTable;
class NotificationBoard;
class IARPCache;

// ICMP type 2, code 4: fragmentation needed, but don't-fragment bit set
const int ICMP_FRAGMENTATION_ERROR_CODE = 4;


/**
 * Implements the IPv4 protocol.
 */
class INET_API IPv4 : public QueueBase, public ILifecycle
{
  protected:
    IIPv4RoutingTable *rt;
    IInterfaceTable *ift;
    IARPCache *arp;
    NotificationBoard *nb;
    ICMPAccess icmpAccess;
    bool manetRouting;
    cGate *arpDgramOutGate;
    cGate *arpInGate;
    cGate *arpOutGate;
    int transportInGateBaseId;
    int queueOutGateBaseId;

    // config
    int defaultTimeToLive;
    int defaultMCTimeToLive;
    simtime_t fragmentTimeoutTime;
    bool forceBroadcast;

    // working vars
    bool isUp;
    long curFragmentId; // counter, used to assign unique fragmentIds to datagrams
    IPv4FragBuf fragbuf;  // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    ProtocolMapping mapping; // where to send packets after decapsulation

    // statistics
    int numMulticast;
    int numLocalDeliver;
    int numDropped;  // forwarding off, no outgoing interface, too large but "don't fragment" is set, TTL exceeded, etc
    int numUnroutable;
    int numForwarded;


  protected:
    // utility: look up interface from getArrivalGate()
    virtual const InterfaceEntry *getSourceInterfaceFrom(cPacket *packet);

    // utility: look up route to the source of the datagram and return its interface
    virtual InterfaceEntry *getShortestPathInterfaceToSource(IPv4Datagram *datagram);

    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    /**
     * Encapsulate packet coming from higher layers into IPv4Datagram, using
     * the given control info. Override if you subclassed controlInfo and/or
     * want to add options etc to the datagram.
     */
    virtual IPv4Datagram *encapsulate(cPacket *transportPacket, IPv4ControlInfo *controlInfo);

    /**
     * Creates a blank IPv4 datagram. Override when subclassing IPv4Datagram is needed
     */
    virtual IPv4Datagram *createIPv4Datagram(const char *name);

    /**
     * Handle IPv4Datagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handleIncomingDatagram(IPv4Datagram *datagram, const InterfaceEntry *fromIE);

    /**
     * Handle messages (typically packets to be send in IPv4) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handlePacketFromHL(cPacket *packet);

    /**
     * TODO
     */
    virtual void handlePacketFromARP(cPacket *packet);

    /**
     * Handle incoming ARP packets by sending them over to ARP.
     */
    virtual void handleIncomingARPPacket(ARPPacket *packet, const InterfaceEntry *fromIE);

    /**
     * Handle incoming ICMP messages.
     */
    virtual void handleIncomingICMP(ICMPMessage *packet);

    /**
     * Performs unicast routing. Based on the routing decision, it sends the
     * datagram through the outgoing interface.
     */
    virtual void routeUnicastPacket(IPv4Datagram *datagram, InterfaceEntry *destIE, IPv4Address nextHopAddr);

    /**
     * Broadcasts the datagram on the specified interface.
     * When destIE is NULL, the datagram is broadcasted on each interface.
     */
    virtual void routeLocalBroadcastPacket(IPv4Datagram *datagram, InterfaceEntry *destIE);

    /**
     * Determines the output interface for the given multicast datagram.
     */
    virtual InterfaceEntry *determineOutgoingInterfaceForMulticastDatagram(IPv4Datagram *datagram, InterfaceEntry *multicastIFOption);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void forwardMulticastPacket(IPv4Datagram *datagram, InterfaceEntry *fromIE);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void reassembleAndDeliver(IPv4Datagram *datagram);

    /**
     * Decapsulate and return encapsulated packet after attaching IPv4ControlInfo.
     */
    virtual cPacket *decapsulate(IPv4Datagram *datagram);

    /**
     * Fragment packet if needed, then send it to the selected interface using
     * sendDatagramToOutput().
     */
    virtual void fragmentAndSend(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr);

    /**
     * Send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(IPv4Datagram *datagram, InterfaceEntry *ie, IPv4Address nextHopAddr);

    virtual MACAddress resolveNextHopMacAddress(cPacket *packet, IPv4Address nextHopAddr, const InterfaceEntry *destIE);

    virtual void sendPacketToIeee802NIC(cPacket *packet, const InterfaceEntry *ie, const MACAddress& macAddress, int etherType);

    virtual void sendPacketToNIC(cPacket *packet, const InterfaceEntry *ie);

  public:
    IPv4() { rt = NULL; ift = NULL; arp = NULL; arpOutGate = NULL; }

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    /**
     * Processing of IPv4 datagrams. Called when a datagram reaches the front
     * of the queue.
     */
    virtual void endService(cPacket *packet);

#ifdef WITH_MANET
    /**
     * Sends a MANET_ROUTE_UPDATE packet to Manet. The datagram is
     * not transmitted, only its source and destination address is used.
     * About DSR datagrams no update message is sent.
     */
    virtual void sendRouteUpdateMessageToManet(IPv4Datagram *datagram);

    /**
     * Sends a MANET_ROUTE_NOROUTE packet to Manet. The packet
     * will encapsulate the given datagram, so this method takes
     * ownership.
     * DSR datagrams are transmitted as they are, i.e. without
     * encapsulation. (?)
     */
    virtual void sendNoRouteMessageToManet(IPv4Datagram *datagram);

    /**
     * Sends a packet to the Manet module.
     */
    virtual void sendToManet(cPacket *packet);
#endif

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    virtual bool isNodeUp();
    virtual void stop();
    virtual void start();
    virtual void flush();
};

#endif

