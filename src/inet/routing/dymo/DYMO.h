//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

#ifndef __INET_DYMO_H
#define __INET_DYMO_H

#include <vector>
#include <map>
#include <omnetpp.h>
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/udp/UdpHeader.h"
#include "inet/routing/dymo/DYMOdefs.h"
#include "inet/routing/dymo/DYMORouteData.h"
#include "inet/routing/dymo/DYMO_m.h"

namespace inet {

namespace dymo {

/**
 * This class provides Dynamic MANET On-demand (DYMO also known as AODVv2) Routing
 * based on the IETF draft at http://tools.ietf.org/html/draft-ietf-manet-dymo-24.
 *
 * Optional features implemented:
 *  - 7.1. Route Discovery Retries and Buffering
 *    To reduce congestion in a network, repeated attempts at route
      discovery for a particular Target Node SHOULD utilize a binary
      exponential backoff.
 *  - 13.1. Expanding Rings Multicast
 *    Increase hop limit from min to max with each retry.
 *  - 13.2. Intermediate RREP
 *    Allow intermediate DYMO routers to reply with RREP.
 *  - 13.6. Message Aggregation
 *    RFC5148 add jitter to broadcasts
 */
class INET_API DYMO : public cSimpleModule, public ILifecycle, public cListener, public NetfilterBase::HookBase
{
  private:
    // DYMO parameters from RFC
    const char *clientAddresses;
    bool useMulticastRREP;
    const char *interfaces;
    double activeInterval;
    double maxIdleTime;
    double maxSequenceNumberLifetime;
    double routeRREQWaitTime;
    double rreqHolddownTime;
    int maxHopCount;
    int discoveryAttemptsMax;
    bool appendInformation;
    int bufferSizePackets;
    int bufferSizeBytes;

    // DYMO extension parameters
    simtime_t maxJitter;
    bool sendIntermediateRREP;
    int minHopLimit;
    int maxHopLimit;

    // context
    cModule *host;
    NodeStatus *nodeStatus;
    IL3AddressType *addressType;
    IInterfaceTable *interfaceTable;
    IRoutingTable *routingTable;
    INetfilter *networkProtocol;

    // internal
    cMessage *expungeTimer;
    DYMOSequenceNumber sequenceNumber;
    std::map<L3Address, DYMOSequenceNumber> targetAddressToSequenceNumber;
    std::map<L3Address, RREQTimer *> targetAddressToRREQTimer;
    std::multimap<L3Address, std::pair<Packet *, std::shared_ptr<IPv4Header>>> targetAddressToDelayedPackets;
    std::vector<std::pair<L3Address, int> > clientAddressAndPrefixLengthPairs;    // 5.3.  Router Clients and Client Networks

  public:
    DYMO();
    virtual ~DYMO();

  protected:
    // module interface
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(cMessage *message) override;

  private:
    // handling messages
    void processSelfMessage(cMessage *message);
    void processMessage(cMessage *message);

    // route discovery
    void startRouteDiscovery(const L3Address& target);
    void retryRouteDiscovery(const L3Address& target, int retryCount);
    void completeRouteDiscovery(const L3Address& target);
    void cancelRouteDiscovery(const L3Address& target);
    bool hasOngoingRouteDiscovery(const L3Address& target);

    // handling IP datagrams
    void delayDatagram(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header);
    void reinjectDelayedDatagram(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header);
    void dropDelayedDatagram(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header);
    void eraseDelayedDatagrams(const L3Address& target);
    bool hasDelayedDatagrams(const L3Address& target);

    // handling RREQ timers
    void cancelRREQTimer(const L3Address& target);
    void deleteRREQTimer(const L3Address& target);
    void eraseRREQTimer(const L3Address& target);

    // handling RREQ wait RREP timer
    RREQWaitRREPTimer *createRREQWaitRREPTimer(const L3Address& target, int retryCount);
    void scheduleRREQWaitRREPTimer(RREQWaitRREPTimer *message);
    void processRREQWaitRREPTimer(RREQWaitRREPTimer *message);

    // handling RREQ backoff timer
    RREQBackoffTimer *createRREQBackoffTimer(const L3Address& target, int retryCount);
    void scheduleRREQBackoffTimer(RREQBackoffTimer *message);
    void processRREQBackoffTimer(RREQBackoffTimer *message);
    simtime_t computeRREQBackoffTime(int retryCount);

    // handling RREQ holddown timer
    RREQHolddownTimer *createRREQHolddownTimer(const L3Address& target);
    void scheduleRREQHolddownTimer(RREQHolddownTimer *message);
    void processRREQHolddownTimer(RREQHolddownTimer *message);

    // handling UDP packets
    void sendUDPPacket(cPacket *packet, double delay);
    void processUDPPacket(Packet *packet);

    // handling DYMO packets
    void sendDYMOPacket(const std::shared_ptr<DYMOPacket>& packet, const InterfaceEntry *interfaceEntry, const L3Address& nextHop, double delay);
    void processDYMOPacket(Packet *packet, const std::shared_ptr<DYMOPacket>& dymoPacket);

    // handling RteMsg packets
    bool permissibleRteMsg(Packet *packet, const std::shared_ptr<RteMsg>& rteMsg);
    void processRteMsg(Packet *packet, const std::shared_ptr<RteMsg>& rteMsg);
    bit computeRteMsgLength(const std::shared_ptr<RteMsg>& rteMsg);

    // handling RREQ packets
    std::shared_ptr<RREQ> createRREQ(const L3Address& target, int retryCount);
    void sendRREQ(const std::shared_ptr<RREQ>& rreq);
    void processRREQ(Packet *packet, const std::shared_ptr<RREQ>& rreq);
    bit computeRREQLength(const std::shared_ptr<RREQ> &rreq);

    // handling RREP packets
    std::shared_ptr<RREP> createRREP(const std::shared_ptr<RteMsg>& rteMsg);
    std::shared_ptr<RREP> createRREP(const std::shared_ptr<RteMsg>& rteMsg, IRoute *route);
    void sendRREP(const std::shared_ptr<RREP>& rrep);
    void sendRREP(const std::shared_ptr<RREP>& rrep, IRoute *route);
    void processRREP(Packet *packet, const std::shared_ptr<RREP>& rrep);
    bit computeRREPLength(const std::shared_ptr<RREP> &rrep);

    // handling RERR packets
    std::shared_ptr<RERR> createRERR(std::vector<L3Address>& addresses);
    void sendRERR(const std::shared_ptr<RERR>& rerr);
    void sendRERRForUndeliverablePacket(const L3Address& destination);
    void sendRERRForBrokenLink(const InterfaceEntry *interfaceEntry, const L3Address& nextHop);
    void processRERR(Packet *packet, const std::shared_ptr<RERR>& rerr);
    bit computeRERRLength(const std::shared_ptr<RERR>& rerr);

    // handling routes
    IRoute *createRoute(Packet *packet, const std::shared_ptr<RteMsg>& rteMsg, AddressBlock& addressBlock);
    void updateRoutes(Packet *packet, const std::shared_ptr<RteMsg>& rteMsg, AddressBlock& addressBlock);
    void updateRoute(Packet *packet, const std::shared_ptr<RteMsg>& rteMsg, AddressBlock& addressBlock, IRoute *route);
    int getLinkCost(const InterfaceEntry *interfaceEntry, DYMOMetricType metricType);
    bool isLoopFree(const std::shared_ptr<RteMsg>& rteMsg, IRoute *route);

    // handling expunge timer
    void processExpungeTimer();
    void scheduleExpungeTimer();
    void expungeRoutes();
    simtime_t getNextExpungeTime();
    DYMORouteState getRouteState(DYMORouteData *routeData);

    // configuration
    bool isNodeUp();
    void configureInterfaces();

    // address
    L3Address getSelfAddress();
    bool isClientAddress(const L3Address& address);

    // added node
    void addSelfNode(const std::shared_ptr<RteMsg>& rteMsg);
    void addNode(const std::shared_ptr<RteMsg>& rteMsg, AddressBlock& addressBlock);

    // sequence number
    void incrementSequenceNumber();

    // routing
    Result ensureRouteForDatagram(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header);

    // netfilter
    virtual Result datagramPreRoutingHook(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) override { Enter_Method("datagramPreRoutingHook"); return ensureRouteForDatagram(datagram, ipv4Header); }
    virtual Result datagramForwardHook(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *datagram, const std::shared_ptr<IPv4Header>& ipv4Header, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress) override { Enter_Method("datagramLocalOutHook"); return ensureRouteForDatagram(datagram, ipv4Header); }

    // lifecycle
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    // notification
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace dymo

} // namespace inet

#endif // ifndef __INET_DYMO_H

