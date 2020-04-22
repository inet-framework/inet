//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IPV4_H
#define __INET_IPV4_H

#include <list>
#include <map>
#include <set>

#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/Ipv4FragBuf.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

class ArpPacket;
class IcmpHeader;
class IInterfaceTable;
class IIpv4RoutingTable;

/**
 * Implements the Ipv4 protocol.
 */
class INET_API Ipv4 : public OperationalBase, public NetfilterBase, public INetworkProtocol, public IProtocolRegistrationListener, public cListener
{
  public:
    /**
     * Represents an Ipv4Header, queued by a Hook
     */
    class QueuedDatagramForHook
    {
      public:
        QueuedDatagramForHook(Packet *packet, IHook::Type hookType) :
            packet(packet), hookType(hookType) {}
        virtual ~QueuedDatagramForHook() {}

        Packet *packet = nullptr;
        const IHook::Type hookType = static_cast<IHook::Type>(-1);
    };
    typedef std::map<Ipv4Address, cPacketQueue> PendingPackets;

    struct SocketDescriptor
    {
        int socketId = -1;
        int protocolId = -1;
        Ipv4Address localAddress;
        Ipv4Address remoteAddress;

        SocketDescriptor(int socketId, int protocolId, Ipv4Address localAddress)
                : socketId(socketId), protocolId(protocolId), localAddress(localAddress) { }
    };

  protected:
    IIpv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    IArp *arp = nullptr;
    Icmp *icmp = nullptr;
    int transportInGateBaseId = -1;

    // config
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int defaultTimeToLive = -1;
    int defaultMCTimeToLive = -1;
    simtime_t fragmentTimeoutTime;
    bool limitedBroadcast = false;
    std::string directBroadcastInterfaces = "";

    cPatternMatcher directBroadcastInterfaceMatcher;

    // working vars
    uint16_t curFragmentId = -1;    // counter, used to assign unique fragmentIds to datagrams
    Ipv4FragBuf fragbuf;    // fragmentation reassembly buffer
    simtime_t lastCheckTime;    // when fragbuf was last checked for state fragments
    std::set<const Protocol *> upperProtocols;    // where to send packets after decapsulation
    std::map<int, SocketDescriptor *> socketIdToSocketDescriptor;

    // ARP related
    PendingPackets pendingPackets;    // map indexed with IPv4Address for outbound packets waiting for ARP resolution

    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0;    // forwarding off, no outgoing interface, too large but "don't fragment" is set, TTL exceeded, etc
    int numUnroutable = 0;
    int numForwarded = 0;

    // hooks
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual const InterfaceEntry *getSourceInterface(Packet *packet);
    virtual const InterfaceEntry *getDestInterface(Packet *packet);
    virtual Ipv4Address getNextHop(Packet *packet);

    // utility: look up route to the source of the datagram and return its interface
    virtual const InterfaceEntry *getShortestPathInterfaceToSource(const Ptr<const Ipv4Header>& ipv4Header) const;

    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    // utility: processing requested ARP resolution completed
    void arpResolutionCompleted(IArp::Notification *entry);

    // utility: processing requested ARP resolution timed out
    void arpResolutionTimedOut(IArp::Notification *entry);

    // utility: verifying CRC
    bool verifyCrc(const Ptr<const Ipv4Header>& ipv4Header);

    // utility: calculate and set CRC
    void setComputedCrc(Ptr<Ipv4Header>& ipv4Header);

  public:
    static void insertCrc(const Ptr<Ipv4Header>& ipv4Header);

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
    virtual const InterfaceEntry *determineOutgoingInterfaceForMulticastDatagram(const Ptr<const Ipv4Header>& ipv4Header, const InterfaceEntry *multicastIFOption);

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

    virtual MacAddress resolveNextHopMacAddress(cPacket *packet, Ipv4Address nextHopAddr, const InterfaceEntry *destIE);

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

  protected:
    /**
     * called before a packet arriving from the network is routed
     */
    IHook::Result datagramPreRoutingHook(Packet *datagram);

    /**
     * called before a packet arriving from the network is delivered via the network
     */
    IHook::Result datagramForwardHook(Packet *datagram);

    /**
     * called before a packet is delivered via the network
     */
    IHook::Result datagramPostRoutingHook(Packet *datagram);

    /**
     * called before a packet arriving from the network is delivered locally
     */
    IHook::Result datagramLocalInHook(Packet *datagram);

    /**
     * called before a packet arriving locally is delivered
     */
    IHook::Result datagramLocalOutHook(Packet *datagram);

  public:
    /**
     * registers a Hook to be executed during datagram processing
     */
    virtual void registerHook(int priority, IHook *hook) override;

    /**
     * unregisters a Hook to be executed during datagram processing
     */
    virtual void unregisterHook(IHook *hook) override;

    /**
     * drop a previously queued datagram
     */
    virtual void dropQueuedDatagram(const Packet *datagram) override;

    /**
     * re-injects a previously queued datagram
     */
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;

    /**
     * ILifecycle methods
     */
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
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

#endif // ifndef __INET_IPV4_H

