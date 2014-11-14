//
// Copyright (C) 2012 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_GENERICNETWORKPROTOCOL_H
#define __INET_GENERICNETWORKPROTOCOL_H

#include <list>
#include <map>

#include "inet/networklayer/arp/IARP.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/common/queue/QueueBase.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/common/INetfilter.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"
#include "inet/networklayer/generic/GenericDatagram.h"
#include "inet/common/ProtocolMap.h"

namespace inet {

/**
 * Implements a generic network protocol that routes generic datagrams through the network.
 * Routing decisions are based on a generic routing table, but it also supports the netfilter
 * interface to allow routing protocols to kick in. It doesn't provide datagram fragmentation
 * and reassembling.
 */
// TODO: rename this and its friends to something that is more specific
// TODO: that expresses to some extent how this network protocol works
class INET_API GenericNetworkProtocol : public QueueBase, public INetfilter, public INetworkProtocol
{
  protected:
    /**
     * Represents an GenericDatagram, queued by a Hook
     */
    struct QueuedDatagramForHook
    {
      public:
        QueuedDatagramForHook(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *outIE,
                L3Address& nextHop, INetfilter::IHook::Type hookType)
            : datagram(datagram), inIE(inIE), outIE(outIE), nextHop(nextHop), hookType(hookType) {}

        virtual ~QueuedDatagramForHook() {}

        GenericDatagram *datagram;
        const InterfaceEntry *inIE;
        const InterfaceEntry *outIE;
        const L3Address nextHop;
        const INetfilter::IHook::Type hookType;
    };

    IInterfaceTable *interfaceTable;
    GenericRoutingTable *routingTable;
    IARP *arp;
    int queueOutBaseGateId;

    // config
    int defaultHopLimit;

    // working vars
    ProtocolMapping mapping;    // where to send packets after decapsulation

    // hooks
    typedef std::multimap<int, IHook *> HookList;
    HookList hooks;
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

    // statistics
    int numLocalDeliver;
    int numDropped;
    int numUnroutable;
    int numForwarded;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual const InterfaceEntry *getSourceInterfaceFrom(cPacket *packet);

    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    /**
     * Handle GenericDatagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handlePacketFromNetwork(GenericDatagram *datagram);

    /**
     * Handle messages (typically packets to be send in Generic) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(cPacket *packet);

    /**
     * Performs routing. Based on the routing decision, it dispatches to
     * sendDatagramToHL() for local packets, to sendDatagramToOutput() for forwarded packets,
     * to handleMulticastPacket() for multicast packets, or drops the packet if
     * it's unroutable or forwarding is off.
     */
    virtual void routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using sendDatagramToOutput().
     */
    virtual void routeMulticastPacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const InterfaceEntry *fromIE);

    /**
     * Encapsulate packet coming from higher layers into GenericDatagram, using
     * the control info attached to the packet.
     */
    virtual GenericDatagram *encapsulate(cPacket *transportPacket, const InterfaceEntry *& destIE);

    /**
     * Decapsulate and return encapsulated packet after attaching GenericNetworkProtocolControlInfo.
     */
    virtual cPacket *decapsulate(GenericDatagram *datagram);

    /**
     * Send datagrams up to the higher layers.
     */
    virtual void sendDatagramToHL(GenericDatagram *datagram);

    /**
     * Last TTL check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(GenericDatagram *datagram, const InterfaceEntry *ie, const L3Address& nextHop);

    virtual void datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop);
    virtual void datagramLocalIn(GenericDatagram *datagram, const InterfaceEntry *inIE);
    virtual void datagramLocalOut(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop);

    virtual IHook::Result datagramPreRoutingHook(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHop);
    virtual IHook::Result datagramForwardHook(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHop);
    virtual IHook::Result datagramPostRoutingHook(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHop);
    virtual IHook::Result datagramLocalInHook(GenericDatagram *datagram, const InterfaceEntry *inIE);
    virtual IHook::Result datagramLocalOutHook(GenericDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHop);

  public:
    GenericNetworkProtocol();
    ~GenericNetworkProtocol();

    virtual void registerHook(int priority, IHook *hook);
    virtual void unregisterHook(int priority, IHook *hook);
    virtual void dropQueuedDatagram(const INetworkDatagram *datagram);
    virtual void reinjectQueuedDatagram(const INetworkDatagram *datagram);

  protected:
    /**
     * Initialization
     */
    virtual void initialize();

    /**
     * Handle message.
     */
    virtual void handleMessage(cMessage *message);

    /**
     * Processing of generic datagrams. Called when a datagram reaches the front
     * of the queue.
     */
    virtual void endService(cPacket *packet);
};

} // namespace inet

#endif // ifndef __INET_GENERICNETWORKPROTOCOL_H

