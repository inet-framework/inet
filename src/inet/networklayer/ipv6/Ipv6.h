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

#ifndef __INET_IPV6_H
#define __INET_IPV6_H

#include <map>
#include <set>

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
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
class INET_API Ipv6 : public cSimpleModule, public NetfilterBase, public LifecycleUnsupported, public INetworkProtocol, public IProtocolRegistrationListener
{
  public:
    /**
     * Represents an Ipv4Header, queued by a Hook
     */
    class QueuedDatagramForHook
    {
      public:
        QueuedDatagramForHook(Packet *packet, IHook::Type hookType) :
            packet(packet), inIE(nullptr), outIE(nullptr),
            hookType(hookType) {}
        virtual ~QueuedDatagramForHook() {}

        Packet *packet = nullptr;
        const InterfaceEntry *inIE = nullptr;
        const InterfaceEntry *outIE = nullptr;
        Ipv6Address nextHopAddr;
        const IHook::Type hookType = static_cast<IHook::Type>(-1);
    };

  protected:
    struct SocketDescriptor
    {
        int socketId = -1;
        int protocolId = -1;
        Ipv6Address localAddress;
        Ipv6Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv6Address localAddress)
                : socketId(socketId), protocolId(protocolId), localAddress(localAddress) { }
    };

    IInterfaceTable *ift = nullptr;
    Ipv6RoutingTable *rt = nullptr;
    Ipv6NeighbourDiscovery *nd = nullptr;
    Icmpv6 *icmp = nullptr;

    Ipv6Tunneling *tunneling = nullptr;

    // working vars
    unsigned int curFragmentId = -1;    // counter, used to assign unique fragmentIds to datagrams
    Ipv6FragBuf fragbuf;    // fragmentation reassembly buffer
    simtime_t lastCheckTime;    // when fragbuf was last checked for state fragments
    std::set<const Protocol *> upperProtocols;    // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0;
    int numUnroutable = 0;
    int numForwarded = 0;

#ifdef WITH_xMIPv6
    // 28.9.07 - CB
    // datagrams that are supposed to be sent with a tentative Ipv6 address
    // are rescheduled for later resubmission.
    class ScheduledDatagram : public cPacket
    {
      protected:
        Packet *packet = nullptr;
        const Ipv6Header *ipv6Header = nullptr;
        const InterfaceEntry *ie = nullptr;
        MacAddress macAddr;
        bool fromHL = false;
      public:
        ScheduledDatagram(Packet *packet, const Ipv6Header *datagram, const InterfaceEntry *ie, MacAddress macAddr, bool fromHL);
        ~ScheduledDatagram();
        const InterfaceEntry *getIE() { return ie; }
        const Ipv6Address& getSrcAddress() {return ipv6Header->getSrcAddress(); }
        const MacAddress& getMacAddress() { return macAddr; }
        bool getFromHL() { return fromHL; }
        Packet *removeDatagram() { Packet *ret = packet; packet = nullptr; return ret; }
    };
#endif /* WITH_xMIPv6 */

    // netfilter hook variables
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual InterfaceEntry *getSourceInterfaceFrom(Packet *msg);

    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    /**
     * Encapsulate packet coming from higher layers into IPv6Datagram
     */
    virtual void encapsulate(Packet *transportPacket);

    virtual void preroutingFinish(Packet *packet, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, Ipv6Address nextHopAddr);

    virtual void handleMessage(cMessage *msg) override;

    virtual void handleRequest(Request *request);

    /**
     * Handle messages (typically packets to be send in Ipv6) from transport or ICMP.
     * Invokes encapsulate(), then routePacket().
     */
    virtual void handleMessageFromHL(Packet *msg);
    virtual void datagramLocalOut(Packet *packet, const InterfaceEntry *destIE, Ipv6Address requestedNextHopAddress);

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
    virtual void routePacket(Packet *packet, const InterfaceEntry *destIE, const InterfaceEntry *fromIE, Ipv6Address requestedNextHopAddress, bool fromHL);
    virtual void resolveMACAddressAndSendPacket(Packet *packet, int interfaceID, Ipv6Address nextHop, bool fromHL);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void routeMulticastPacket(Packet *packet, const InterfaceEntry *destIE, const InterfaceEntry *fromIE, bool fromHL);

    virtual void fragmentPostRouting(Packet *packet, const InterfaceEntry *ie, const MacAddress& nextHopAddr, bool fromHL);

    /**
     * Performs fragmentation if needed, and sends the original datagram or the fragments
     * through the specified interface.
     */
    virtual void fragmentAndSend(Packet *packet, const InterfaceEntry *destIE, const MacAddress& nextHopAddr, bool fromHL);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void localDeliver(Packet *packet, const InterfaceEntry *fromIE);

    /**
     * Decapsulate packet.
     */
    virtual void decapsulate(Packet *packet);

    /**
     * Last hoplimit check, then send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(Packet *packet, const InterfaceEntry *destIE, const MacAddress& macAddr);

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
    IHook::Result datagramPostRoutingHook(Packet *packet, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered locally
     */
    IHook::Result datagramLocalInHook(Packet *packet, const InterfaceEntry *inIE);

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
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     * Determines the correct interface for the specified destination address.
     * The nextHop and interfaceId are output parameter.
     */
    bool determineOutputInterface(const Ipv6Address& destAddress, Ipv6Address& nextHop, int& interfaceId,
            Packet *packet, bool fromHL);

#ifdef WITH_xMIPv6
    /**
     * Process the extension headers of the datagram.
     * Returns true if all have been processed successfully and false if errors occured
     * and the packet has to be dropped or if the datagram has been forwarded to another
     * module for further processing.
     */
    bool processExtensionHeaders(Packet *packet, const Ipv6Header *ipv6Header);
#endif /* WITH_xMIPv6 */
};

} // namespace inet

#endif // ifndef __INET_IPV6_H

