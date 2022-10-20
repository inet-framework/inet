//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4_H
#define __INET_IPV4_H

#include <list>
#include <map>
#include <set>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/contract/netfilter/INetfilterCompatibleNetfilterHookManagerBase.h"
#include "inet/networklayer/ipv4layer/common/Ipv4FragBuf.h"
#include "inet/networklayer/ipv4layer/common/Ipv4Header_m.h"
#include "inet/networklayer/ipv4layer/icmp/Icmp.h"

namespace inet {

class ArpPacket;
class IcmpHeader;
class IInterfaceTable;
class IIpv4RoutingTable;

/**
 * Implements the Ipv4 protocol.
 */
class INET_API Ipv4 : public OperationalBase, public NetfilterHook::INetfilterCompatibleNetfilterHookManagerBase, public INetworkProtocol, public DefaultProtocolRegistrationListener, public cListener
{
  protected:
    class Item {
      public:
        int priority;
        NetfilterHook::NetfilterHandler *handler;
        Item(int priority, NetfilterHook::NetfilterHandler *handler) : priority(priority), handler(handler) {}
    };
    typedef std::vector<Item> Items;
    Items hooks[NetfilterHook::NetfilterType::__NUM_HOOK_TYPES];

    // ARP related
    typedef std::map<Ipv4Address, cPacketQueue> PendingPackets;

    struct SocketDescriptor {
        int socketId = -1;
        int protocolId = -1;
        Ipv4Address localAddress;
        Ipv4Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv4Address localAddress)
            : socketId(socketId), protocolId(protocolId), localAddress(localAddress) {}
    };

    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IArp> arp;
    ModuleRefByPar<Icmp> icmp;

    // config
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int defaultTimeToLive = -1;
    int defaultMCTimeToLive = -1;
    simtime_t fragmentTimeoutTime;
    bool limitedBroadcast = false;
    std::string directBroadcastInterfaces = "";

    cPatternMatcher directBroadcastInterfaceMatcher;

    // working vars
    uint16_t curFragmentId = -1; // counter, used to assign unique fragmentIds to datagrams
    Ipv4FragBuf fragbuf; // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // ARP related
    PendingPackets pendingPackets; // map indexed with IPv4Address for outbound packets waiting for ARP resolution

    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0; // forwarding off, no outgoing interface, too large but "don't fragment" is set, TTL exceeded, etc
    int numUnroutable = 0;
    int numForwarded = 0;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual const NetworkInterface *getSourceInterface(Packet *packet);
    virtual const NetworkInterface *getDestInterface(Packet *packet);
    virtual Ipv4Address getNextHop(Packet *packet);

    // utility: look up route to the source of the datagram and return its interface
    virtual const NetworkInterface *getShortestPathInterfaceToSource(const Ptr<const Ipv4Header>& ipv4Header) const;

    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    // utility: processing requested ARP resolution completed
    void arpResolutionCompleted(IArp::Notification *entry);

    // utility: processing requested ARP resolution timed out
    void arpResolutionTimedOut(IArp::Notification *entry);

  protected:
    /**
     * Encapsulate packet coming from higher layers into Ipv4Header, using
     * the given control info. Override if you subclassed controlInfo and/or
     * want to add options etc to the datagram.
     */
    virtual void encapsulate(Packet *packet);

    /**
     * Handle Ipv4Header messages arriving from lower layer.
     * Decrements TTL, then invokes routePacket().
     */
    virtual void handleIncomingDatagram(Packet *packet);

    // called after PREROUTING Hook (used for reinject, too)
    virtual void preroutingFinish(Packet *packet);

    /**
     * Handle messages (typically packets to be send in Ipv4) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handlePacketFromHL(Packet *packet);

    /**
     * Routes and sends datagram received from higher layers.
     * Invokes datagramLocalOutHook(), then routePacket().
     */
    virtual void datagramLocalOut(Packet *packet);

    /**
     * Performs unicast routing. Based on the routing decision, it sends the
     * datagram through the outgoing interface.
     */
    virtual void routeUnicastPacket(Packet *packet);

    // called after FORWARD Hook (used for reinject, too)
    void routeUnicastPacketFinish(Packet *packet);

    /**
     * Broadcasts the datagram on the specified interface.
     * When destIE is nullptr, the datagram is broadcasted on each interface.
     */
    virtual void routeLocalBroadcastPacket(Packet *packet);

    /**
     * Determines the output interface for the given multicast datagram.
     */
    virtual const NetworkInterface *determineOutgoingInterfaceForMulticastDatagram(const Ptr<const Ipv4Header>& ipv4Header, const NetworkInterface *multicastIFOption);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void forwardMulticastPacket(Packet *packet);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void reassembleAndDeliver(Packet *packet);

    // called after LOCAL_IN Hook (used for reinject, too)
    virtual void reassembleAndDeliverFinish(Packet *packet);

    /**
     * Decapsulate packet.
     */
    virtual void decapsulate(Packet *packet);

    /**
     * Call PostRouting Hook and continue with fragmentAndSend() if accepted
     */
    virtual void fragmentPostRouting(Packet *datagram);

    /**
     * Fragment packet if needed, then send it to the selected interface using
     * sendDatagramToOutput().
     */
    virtual void fragmentAndSend(Packet *packet);

    /**
     * Send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(Packet *packet);

    virtual MacAddress resolveNextHopMacAddress(cPacket *packet, Ipv4Address nextHopAddr, const NetworkInterface *destIE);

    virtual void sendPacketToNIC(Packet *packet);

    virtual void sendIcmpError(Packet *packet, int inputInterfaceId, IcmpType type, IcmpCode code);

    virtual Packet *prepareForForwarding(Packet *packet) const;

  public:
    Ipv4();
    virtual ~Ipv4();

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void handleRequest(Request *request);

    // NetFilter functions:
    Ipv4::Items::iterator findHookPosition(NetfilterHook::NetfilterType type, int priority, const NetfilterHook::NetfilterHandler *handler);

    /**
     * called before a packet arriving from the network is routed
     */
    NetfilterHook::NetfilterResult processHook(NetfilterHook::NetfilterType type, Packet *datagram, Ipv4::Items::iterator it);
    NetfilterHook::NetfilterResult processHook(NetfilterHook::NetfilterType type, Packet *datagram);

  public:
    /**
     * INetfilterHookManager:
     */
    virtual void registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action) override;

    /**
     * ILifecycle methods
     */
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    /// cListener method
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  protected:
    virtual void start();
    virtual void stop();
    virtual void flush();
};

} // namespace inet

#endif

