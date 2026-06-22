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
#include "inet/networklayer/ipv6/IIpv6ExtensionHeaderHandler.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h"
#include "inet/networklayer/ipv6/Ipv6FragBuf.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

class Icmpv6Header;

/**
 * Ipv6 implementation.
 */
class INET_API Ipv6 : public OperationalBase, public NetfilterBase, public INetworkProtocol, public DefaultProtocolRegistrationListener, protected cListener
{
  public:
    /**
     * Represents an Ipv4Header, queued by a Hook
     */
    class QueuedDatagramForHook {
      public:
        QueuedDatagramForHook(Packet *packet, IHook::Type hookType) :
            packet(packet), inIE(nullptr), outIE(nullptr),
            hookType(hookType) {}
        virtual ~QueuedDatagramForHook() {}

        Packet *packet = nullptr;
        const NetworkInterface *inIE = nullptr;
        const NetworkInterface *outIE = nullptr;
        Ipv6Address nextHopAddr;
        const IHook::Type hookType = static_cast<IHook::Type>(-1);
    };

  protected:
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

    // working vars
    bool sendRedirects = true; // whether to send ICMPv6 Redirects when forwarding back out the arrival interface (RFC 4861 8.2)
    unsigned int curFragmentId = -1; // counter, used to assign unique fragmentIds to datagrams
    Ipv6FragBuf fragbuf; // fragmentation reassembly buffer
    simtime_t lastCheckTime; // when fragbuf was last checked for state fragments
    std::set<const Protocol *> upperProtocols; // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // extension header handler registries
    std::map<int, IIpv6ExtensionHeaderHandler *> routingHeaderHandlers;    // keyed by routing type
    std::map<int, IIpv6TlvOptionHandler *> hopByHopOptionHandlers;         // keyed by TLV option type
    std::map<int, IIpv6TlvOptionHandler *> destOptionHandlers;             // keyed by TLV option type

    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0;
    int numUnroutable = 0;
    int numForwarded = 0;

    // RFC 4862: defer sending when source address is tentative (DAD in progress)
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

    // Queue of datagrams waiting for tentative source address to become permanent
    std::vector<ScheduledDatagram *> pendingDadQueue;

    // netfilter hook variables
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual NetworkInterface *getSourceInterfaceFrom(Packet *msg);

    // utility: next hop requested by a netfilter hook (e.g. a MANET routing
    // protocol) via the NextHopAddressReq tag, or UNSPECIFIED if none
    virtual Ipv6Address getNextHop(Packet *packet);

    // utility: show current statistics above the icon
    virtual std::string getIpv6StatusText() const;

    /**
     * Encapsulate packet coming from higher layers into IPv6Datagram
     */
    virtual void encapsulate(Packet *transportPacket);

    virtual void preroutingFinish(Packet *packet, const NetworkInterface *fromIE, const NetworkInterface *destIE, Ipv6Address nextHopAddr);

    virtual void handleMessageWhenUp(cMessage *msg) override;

    using cListener::receiveSignal;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    virtual void handleRequest(Request *request);

    /**
     * Handle messages (typically packets to be send in Ipv6) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(Packet *msg);

    /**
     * Handle an ICMPv6 error indication arriving from the ICMPv6 module.
     * Pops the quoted IPv6 header from the original packet, converts it
     * to tags, and forwards the indication to the appropriate transport protocol.
     */
    virtual void handleIndication(Indication *indication);
    virtual void handleIcmpErrorIndication(Indication *indication);
    virtual void datagramLocalOut(Packet *packet, const NetworkInterface *destIE, Ipv6Address requestedNextHopAddress);

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
    virtual void routePacket(Packet *packet, const NetworkInterface *destIE, const NetworkInterface *fromIE, Ipv6Address requestedNextHopAddress, bool fromHL);
    virtual void resolveMACAddressAndSendPacket(Packet *packet, int interfaceID, Ipv6Address nextHop, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void routeMulticastPacket(Packet *packet, const NetworkInterface *destIE, const NetworkInterface *fromIE, bool fromHL);

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
    /**
     * called before a packet arriving from the network is routed
     */
    IHook::Result datagramPreRoutingHook(Packet *packet);

    /**
     * called before a packet arriving from the network is delivered via the network
     */
    IHook::Result datagramForwardHook(Packet *packet);

    /**
     * called before a packet is delivered via the network
     */
    IHook::Result datagramPostRoutingHook(Packet *packet, const NetworkInterface *inIE, const NetworkInterface *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered locally
     */
    IHook::Result datagramLocalInHook(Packet *packet, const NetworkInterface *inIE);

    /**
     * called before a packet arriving locally is delivered
     */
    IHook::Result datagramLocalOutHook(Packet *packet);

  public:
    Ipv6();
    ~Ipv6();

    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

    // Netfilter:
    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *packet) override;
    virtual void reinjectQueuedDatagram(const Packet *packet) override;

  protected:
    /**
     * Initialization
     */
    virtual void initialize(int stage) override;

    // lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void start();
    virtual void stop();
    virtual void flush();

    /**
     * Determines the correct interface for the specified destination address.
     * The nextHop and interfaceId are output parameter.
     */
    bool determineOutputInterface(const Ipv6Address& destAddress, Ipv6Address& nextHop, int& interfaceId,
            Packet *packet, bool fromHL);

    /**
     * Process the extension headers of the datagram.
     * Returns true if all have been processed successfully and false if errors occured
     * and the packet has to be dropped or if the datagram has been forwarded to another
     * module for further processing.
     */
    bool processExtensionHeaders(Packet *packet);

    /**
     * Applies RFC 8200 Section 4.2 handling for a Hop-by-Hop or Destination
     * option whose type has no registered handler. The action is encoded in the
     * two highest-order bits of the option type. Returns true if the option was
     * skipped (processing continues), false if the packet was discarded (and
     * possibly an ICMPv6 Parameter Problem error was sent).
     */
    bool handleUnrecognizedTlvOption(Packet *packet, int optType);

  public:
    /** @name Extension header handler registration */
    //@{
    /**
     * Register a handler for a specific Routing Header type (e.g. type 2 for MIPv6).
     * Called during initialization by modules that process routing headers.
     */
    void registerRoutingHeaderHandler(int routingType, IIpv6ExtensionHeaderHandler *handler);

    /**
     * Register a handler for a specific TLV option type within Hop-by-Hop Options headers.
     */
    void registerHopByHopOptionHandler(int optionType, IIpv6TlvOptionHandler *handler);

    /**
     * Register a handler for a specific TLV option type within Destination Options headers
     * (e.g. Home Address Option 0xC9 for MIPv6).
     */
    void registerDestinationOptionHandler(int optionType, IIpv6TlvOptionHandler *handler);
    //@}
};

} // namespace inet

#endif

