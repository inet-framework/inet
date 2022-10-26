//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6_H
#define __INET_IPV6_H

#include <map>
#include <set>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/contract/netfilter/INetfilterCompatibleNetfilterHookManagerBase.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h"
#include "inet/networklayer/ipv6/Ipv6FragBuf.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6tunneling/Ipv6Tunneling.h"

namespace inet {

class Icmpv6Header;

/**
 * Ipv6 implementation.
 */
class INET_API Ipv6 : public cSimpleModule, public NetfilterHook::INetfilterCompatibleNetfilterHookManagerBase, public LifecycleUnsupported, public INetworkProtocol, public DefaultProtocolRegistrationListener
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

    class SocketDescriptor {
      public:
        int socketId = -1;
        int protocolId = -1;
        Ipv6Address localAddress;
        Ipv6Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv6Address localAddress)
            : socketId(socketId), protocolId(protocolId), localAddress(localAddress) {}
    };

    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<Ipv6RoutingTable> rt;
    ModuleRefByPar<Ipv6NeighbourDiscovery> nd;
    ModuleRefByPar<Icmpv6> icmp;
    ModuleRefByPar<Ipv6Tunneling> tunneling;

    // working vars
    unsigned int curFragmentId = -1; // counter, used to assign unique fragmentIds to datagrams
    Ipv6FragBuf fragbuf; // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0;
    int numUnroutable = 0;
    int numForwarded = 0;

#ifdef INET_WITH_xMIPv6
    // 28.9.07 - CB
    // datagrams that are supposed to be sent with a tentative Ipv6 address
    // are rescheduled for later resubmission.
    class INET_API ScheduledDatagram : public cPacket {
      protected:
        Packet *packet = nullptr;
        const Ipv6Header *ipv6Header = nullptr;
        const NetworkInterface *ie = nullptr;
        MacAddress macAddr;
        bool fromHL = false;

      public:
        ScheduledDatagram(Packet *packet, const Ipv6Header *datagram, const NetworkInterface *ie, MacAddress macAddr, bool fromHL);
        ~ScheduledDatagram();
        const NetworkInterface *getIE() { return ie; }
        const Ipv6Address& getSrcAddress() { return ipv6Header->getSrcAddress(); }
        const MacAddress& getMacAddress() { return macAddr; }
        bool getFromHL() { return fromHL; }
        Packet *removeDatagram() { Packet *ret = packet; packet = nullptr; return ret; }
    };
#endif /* INET_WITH_xMIPv6 */

  protected:
    // utility: look up interface from getArrivalGate()
    NetworkInterface *getSourceInterfaceFrom(Packet *msg);
    const NetworkInterface *getDestInterface(Packet *packet);
    Ipv6Address getNextHop(Packet *packet);

    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    /**
     * Encapsulate packet coming from higher layers into IPv6Datagram
     */
    virtual void encapsulate(Packet *transportPacket);

    virtual void preroutingFinish(Packet *packet);

    virtual void handleMessage(cMessage *msg) override;

    virtual void handleRequest(Request *request);

    /**
     * Handle messages (typically packets to be send in Ipv6) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(Packet *msg);

    virtual void datagramLocalOut(Packet *packet);

    /**
     * Handle incoming ICMP messages.
     */
    virtual void handleReceivedIcmp(Packet *msg);

    /**
     * Performs routing. Based on the routing decision, it dispatches to
     * localDeliver() for local packets, to fragmentAndSend() for forwarded packets,
     * to routeMulticastPacket() for multicast packets, or drops the packet if
     * it's unroutable or forwarding is off.
     */
    virtual void routePacket(Packet *packet, bool fromHL);
    virtual void resolveMACAddressAndSendPacket(Packet *packet, int interfaceID, Ipv6Address nextHop, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void routeMulticastPacket(Packet *packet, bool fromHL);

    virtual void fragmentPostRouting(Packet *packet, const NetworkInterface *ie, const MacAddress& nextHopAddr, bool fromHL);

    /**
     * Performs fragmentation if needed, and sends the original datagram or the fragments
     * through the specified interface.
     */
    virtual void fragmentAndSend(Packet *packet, const NetworkInterface *destIE, const MacAddress& nextHopAddr, bool fromHL);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void localDeliver(Packet *packet, const NetworkInterface *fromIE);

    /**
     * Decapsulate packet.
     */
    virtual void decapsulate(Packet *packet);

    /**
     * Last hoplimit check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(Packet *packet, const NetworkInterface *destIE, const MacAddress& macAddr);

    void sendIcmpError(Packet *origPacket, Icmpv6Type type, int code);

    // NetFilter functions:

  protected:
    // NetFilter functions:
    Ipv6::Items::iterator findHookPosition(NetfilterHook::NetfilterType type, int priority, const NetfilterHook::NetfilterHandler *handler);

    /**
     * called before a packet arriving from the network is routed
     */
    NetfilterHook::NetfilterResult processHook(NetfilterHook::NetfilterType type, Packet *datagram, Ipv6::Items::iterator it);
    NetfilterHook::NetfilterResult processHook(NetfilterHook::NetfilterType type, Packet *datagram);

  public:
    Ipv6();
    ~Ipv6();

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

    // Netfilter:
    virtual void registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action) override;

  protected:
    /**
     * Initialization
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     * Determines the correct interface for the specified destination address.
     * The nextHop and interfaceId are output parameter.
     */
    bool determineOutputInterface(const Ipv6Address& destAddress, Ipv6Address& nextHop, int& interfaceId,
            Packet *packet, bool fromHL);

#ifdef INET_WITH_xMIPv6
    /**
     * Process the extension headers of the datagram.
     * Returns true if all have been processed successfully and false if errors occured
     * and the packet has to be dropped or if the datagram has been forwarded to another
     * module for further processing.
     */
    bool processExtensionHeaders(Packet *packet, const Ipv6Header *ipv6Header);
#endif /* INET_WITH_xMIPv6 */
};

} // namespace inet

#endif

