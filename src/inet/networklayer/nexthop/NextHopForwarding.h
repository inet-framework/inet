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

#ifndef __INET_NEXTHOPFORWARDING_H
#define __INET_NEXTHOPFORWARDING_H

#include <list>
#include <map>
#include <set>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"
#include "inet/networklayer/nexthop/NextHopRoutingTable.h"

namespace inet {

/**
 * Implements a next hop forwarding protocol that routes datagrams through the network.
 * Routing decisions are based on a next hop routing table, but it also supports the netfilter
 * interface to allow routing protocols to kick in. It doesn't provide datagram fragmentation
 * and reassembling.
 */
class INET_API NextHopForwarding : public OperationalBase, public NetfilterBase, public INetworkProtocol, public IProtocolRegistrationListener
{
  protected:
    /**
     * Represents an NextHopDatagram, queued by a Hook
     */
    struct QueuedDatagramForHook
    {
      public:
        QueuedDatagramForHook(Packet *datagram, INetfilter::IHook::Type hookType)
            : datagram(datagram), hookType(hookType) {}

        virtual ~QueuedDatagramForHook() {}

        Packet *datagram;
        const InterfaceEntry *inIE;
        const InterfaceEntry *outIE;
        const L3Address nextHop;
        const INetfilter::IHook::Type hookType;
    };

    struct SocketDescriptor
    {
        int socketId = -1;
        int protocolId = -1;
        L3Address localAddress;
        L3Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, L3Address localAddress)
                : socketId(socketId), protocolId(protocolId), localAddress(localAddress) { }
    };

    IInterfaceTable *interfaceTable;
    NextHopRoutingTable *routingTable;
    IArp *arp;

    // config
    int defaultHopLimit;

    // working vars
    std::set<const Protocol *> upperProtocols;    // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // hooks
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

    // statistics
    int numLocalDeliver;
    int numDropped;
    int numUnroutable;
    int numForwarded;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual const InterfaceEntry *getSourceInterfaceFrom(Packet *packet);

    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    /**
     * Handle NextHopDatagram messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handlePacketFromNetwork(Packet *datagram);

    /**
     * Handle packets from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handlePacketFromHL(Packet *packet);

    /**
     * Performs routing. Based on the routing decision, it dispatches to
     * sendDatagramToHL() for local packets, to sendDatagramToOutput() for forwarded packets,
     * to handleMulticastPacket() for multicast packets, or drops the packet if
     * it's unroutable or forwarding is off.
     */
    virtual void routePacket(Packet *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using sendDatagramToOutput().
     */
    virtual void routeMulticastPacket(Packet *datagram, const InterfaceEntry *destIE, const InterfaceEntry *fromIE);

    /**
     * Encapsulate packet coming from higher layers into NextHopDatagram, using
     * the control info attached to the packet.
     */
    virtual void encapsulate(Packet *transportPacket, const InterfaceEntry *& destIE);

    /**
     * Decapsulate and return encapsulated packet.
     */
    virtual void decapsulate(Packet *datagram);

    /**
     * Send datagrams up to the higher layers.
     */
    virtual void sendDatagramToHL(Packet *datagram);

    /**
     * Last TTL check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(Packet *datagram, const InterfaceEntry *ie, L3Address nextHop);

    virtual void datagramPreRouting(Packet *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop);
    virtual void datagramLocalIn(Packet *datagram, const InterfaceEntry *inIE);
    virtual void datagramLocalOut(Packet *datagram, const InterfaceEntry *destIE, const L3Address& nextHop);

    virtual IHook::Result datagramPreRoutingHook(Packet *datagram);
    virtual IHook::Result datagramForwardHook(Packet *datagram);
    virtual IHook::Result datagramPostRoutingHook(Packet *datagram);
    virtual IHook::Result datagramLocalInHook(Packet *datagram);
    virtual IHook::Result datagramLocalOutHook(Packet *datagram);

  public:
    NextHopForwarding();
    ~NextHopForwarding();

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *datagram) override;
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;

  protected:
    /**
     * Initialization
     */
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void handleCommand(Request *msg);

    /**
     * ILifecycle method
     */
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  protected:
    virtual void stop();
    virtual void start();
    virtual void flush();
};

} // namespace inet

#endif // ifndef __INET_NEXTHOPFORWARDING_H

