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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/arp/IARP.h"
#include "inet/networklayer/ipv4/ICMP.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/common/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4FragBuf.h"
#include "inet/common/ProtocolMap.h"
#include "inet/common/queue/QueueBase.h"

namespace inet {

class ARPPacket;
class ICMPMessage;
class IInterfaceTable;
class IIPv4RoutingTable;

/**
 * Implements the IPv4 protocol.
 */
class INET_API IPv4 : public QueueBase, public INetfilter, public ILifecycle, public INetworkProtocol, public cListener
{
  public:
    /**
     * Represents an IPv4Datagram, queued by a Hook
     */
    class QueuedDatagramForHook
    {
      public:
        QueuedDatagramForHook(IPv4Datagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *outIE, const IPv4Address& nextHopAddr, IHook::Type hookType) :
            datagram(datagram), inIE(inIE), outIE(outIE), nextHopAddr(nextHopAddr), hookType(hookType) {}
        virtual ~QueuedDatagramForHook() {}

        IPv4Datagram *datagram;
        const InterfaceEntry *inIE;
        const InterfaceEntry *outIE;
        IPv4Address nextHopAddr;
        const IHook::Type hookType;
    };
    typedef std::map<IPv4Address, cPacketQueue> PendingPackets;

  protected:
    IIPv4RoutingTable *rt;
    IInterfaceTable *ift;
    IARP *arp;
    ICMP *icmp;
    cGate *arpInGate;
    cGate *arpOutGate;
    int transportInGateBaseId;
    int queueOutGateBaseId;

    // config
    int defaultTimeToLive;
    int defaultMCTimeToLive;
    simtime_t fragmentTimeoutTime;
    bool forceBroadcast;
    bool useProxyARP;

    // working vars
    bool isUp;
    long curFragmentId;    // counter, used to assign unique fragmentIds to datagrams
    IPv4FragBuf fragbuf;    // fragmentation reassembly buffer
    simtime_t lastCheckTime;    // when fragbuf was last checked for state fragments
    ProtocolMapping mapping;    // where to send packets after decapsulation

    // ARP related
    PendingPackets pendingPackets;    // map indexed with IPv4Address for outbound packets waiting for ARP resolution

    // statistics
    int numMulticast;
    int numLocalDeliver;
    int numDropped;    // forwarding off, no outgoing interface, too large but "don't fragment" is set, TTL exceeded, etc
    int numUnroutable;
    int numForwarded;

    // hooks
    typedef std::multimap<int, IHook *> HookList;
    HookList hooks;
    typedef std::list<QueuedDatagramForHook> DatagramQueueForHooks;
    DatagramQueueForHooks queuedDatagramsForHooks;

  protected:
    // utility: look up interface from getArrivalGate()
    virtual const InterfaceEntry *getSourceInterfaceFrom(cPacket *packet);

    // utility: look up route to the source of the datagram and return its interface
    virtual const InterfaceEntry *getShortestPathInterfaceToSource(IPv4Datagram *datagram);

    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    // utility: processing requested ARP resolution completed
    void arpResolutionCompleted(IARP::Notification *entry);

    // utility: processing requested ARP resolution timed out
    void arpResolutionTimedOut(IARP::Notification *entry);

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

    // called after PREROUTING Hook (used for reinject, too)
    virtual void preroutingFinish(IPv4Datagram *datagram, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, IPv4Address nextHopAddr);

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
     * Routes and sends datagram received from higher layers.
     * Invokes datagramLocalOutHook(), then routePacket().
     */
    virtual void datagramLocalOut(IPv4Datagram *datagram, const InterfaceEntry *destIE, IPv4Address nextHopAddr);

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
    virtual void routeUnicastPacket(IPv4Datagram *datagram, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, IPv4Address requestedNextHopAddress);

    // called after FORWARD Hook (used for reinject, too)
    void routeUnicastPacketFinish(IPv4Datagram *datagram, const InterfaceEntry *fromIE, const InterfaceEntry *destIE, IPv4Address nextHopAddr);

    /**
     * Broadcasts the datagram on the specified interface.
     * When destIE is NULL, the datagram is broadcasted on each interface.
     */
    virtual void routeLocalBroadcastPacket(IPv4Datagram *datagram, const InterfaceEntry *destIE);

    /**
     * Determines the output interface for the given multicast datagram.
     */
    virtual const InterfaceEntry *determineOutgoingInterfaceForMulticastDatagram(IPv4Datagram *datagram, const InterfaceEntry *multicastIFOption);

    /**
     * Forwards packets to all multicast destinations, using fragmentAndSend().
     */
    virtual void forwardMulticastPacket(IPv4Datagram *datagram, const InterfaceEntry *fromIE);

    /**
     * Perform reassembly of fragmented datagrams, then send them up to the
     * higher layers using sendToHL().
     */
    virtual void reassembleAndDeliver(IPv4Datagram *datagram);

    // called after LOCAL_IN Hook (used for reinject, too)
    virtual void reassembleAndDeliverFinish(IPv4Datagram *datagram);

    /**
     * Decapsulate and return encapsulated packet after attaching IPv4ControlInfo.
     */
    virtual cPacket *decapsulate(IPv4Datagram *datagram);

    /**
     * Call PostRouting Hook and continue with fragmentAndSend() if accepted
     */
    virtual void fragmentPostRouting(IPv4Datagram *datagram, const InterfaceEntry *ie, IPv4Address nextHopAddr);

    /**
     * Fragment packet if needed, then send it to the selected interface using
     * sendDatagramToOutput().
     */
    virtual void fragmentAndSend(IPv4Datagram *datagram, const InterfaceEntry *ie, IPv4Address nextHopAddr);

    /**
     * Send datagram on the given interface.
     */
    virtual void sendDatagramToOutput(IPv4Datagram *datagram, const InterfaceEntry *ie, IPv4Address nextHopAddr);

    virtual MACAddress resolveNextHopMacAddress(cPacket *packet, IPv4Address nextHopAddr, const InterfaceEntry *destIE);

    virtual void sendPacketToIeee802NIC(cPacket *packet, const InterfaceEntry *ie, const MACAddress& macAddress, int etherType);

    virtual void sendPacketToNIC(cPacket *packet, const InterfaceEntry *ie);

  public:
    IPv4();
    virtual ~IPv4();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    /**
     * Processing of IPv4 datagrams. Called when a datagram reaches the front
     * of the queue.
     */
    virtual void endService(cPacket *packet);

    // NetFilter functions:

  protected:
    /**
     * called before a packet arriving from the network is routed
     */
    IHook::Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered via the network
     */
    IHook::Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet is delivered via the network
     */
    IHook::Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

    /**
     * called before a packet arriving from the network is delivered locally
     */
    IHook::Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE);

    /**
     * called before a packet arriving locally is delivered
     */
    IHook::Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr);

  public:
    /**
     * registers a Hook to be executed during datagram processing
     */
    virtual void registerHook(int priority, IHook *hook);

    /**
     * unregisters a Hook to be executed during datagram processing
     */
    virtual void unregisterHook(int priority, IHook *hook);

    /**
     * drop a previously queued datagram
     */
    void dropQueuedDatagram(const INetworkDatagram *datagram);

    /**
     * re-injects a previously queued datagram
     */
    void reinjectQueuedDatagram(const INetworkDatagram *datagram);

    /**
     * send packet on transportOut gate specified by protocolId
     */
    void sendOnTransportOutGateByProtocolId(cPacket *packet, int protocolId);

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    /// cListener method
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

  protected:
    virtual bool isNodeUp();
    virtual void stop();
    virtual void start();
    virtual void flush();
};

} // namespace inet

#endif // ifndef __INET_IPV4_H

