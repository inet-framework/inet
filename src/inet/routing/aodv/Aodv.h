//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AODV_H
#define __INET_AODV_H

#include <map>
#include <deque>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvRouteData.h"
#include "inet/routing/aodv/AodvSequence.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace aodv {

/*
 * This class implements AODV routing protocol and Netfilter hooks
 * in the IP-layer required by this protocol.
 */

class INET_API Aodv : public RoutingProtocolBase, public NetfilterBase::HookBase, public UdpSocket::ICallback, public cListener
{
  protected:
    /*
     * It implements a unique identifier for an arbitrary RREQ message
     * in the network. See: rreqsArrivalTime.
     */
    class INET_API RreqIdentifier {
      public:
        L3Address originatorAddr;
        unsigned int rreqID;
        RreqIdentifier(const L3Address& originatorAddr, unsigned int rreqID) : originatorAddr(originatorAddr), rreqID(rreqID) {};
        bool operator==(const RreqIdentifier& other) const
        {
            return this->originatorAddr == other.originatorAddr && this->rreqID == other.rreqID;
        }
    };

    class INET_API RreqIdentifierCompare {
      public:
        bool operator()(const RreqIdentifier& lhs, const RreqIdentifier& rhs) const
        {
            if (lhs.originatorAddr < rhs.originatorAddr)
                return true;
            else if (lhs.originatorAddr > rhs.originatorAddr)
                return false;
            else
                return lhs.rreqID < rhs.rreqID;
        }
    };

    // context
    const IL3AddressType *addressType = nullptr; // to support both Ipv4 and v6 addresses.

    // environment
    cModule *host = nullptr;
    ModuleRefByPar<IRoutingTable> routingTable;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    ModuleRefByPar<INetfilter> networkProtocol;
    UdpSocket socket;
    NetworkInterface *interface = nullptr;
    std::vector<NetworkInterface *> selectedInterfaces;
    bool usingIpv6 = false;
    long numSent = 0;
    long numReceived = 0;

    // AODV parameters: the following parameters are configurable, see the NED file for more info.
    unsigned int rerrRatelimit = 0;
    unsigned int aodvUDPPort = 0;
    bool askGratuitousRREP = false;
    bool useHelloMessages = false;
    bool destinationOnlyFlag = false;
    simtime_t maxJitter;
    simtime_t activeRouteTimeout;
    simtime_t helloInterval;
    unsigned int netDiameter = 0;
    unsigned int rreqRetries = 0;
    unsigned int rreqRatelimit = 0;
    unsigned int timeoutBuffer = 0;
    unsigned int ttlStart = 0;
    unsigned int ttlIncrement = 0;
    unsigned int ttlThreshold = 0;
    unsigned int localAddTTL = 0;
    unsigned int allowedHelloLoss = 0;
    simtime_t nodeTraversalTime;
    cPar *jitterPar = nullptr;
    cPar *periodicJitter = nullptr;
    L3Address gatewayAddress; // Gateway address for external networks
    bool hasExternalGateway = false;

    // the following parameters are calculated from the parameters defined above
    // see the NED file for more info
    simtime_t deletePeriod;
    simtime_t myRouteTimeout;
    simtime_t blacklistTimeout;
    simtime_t netTraversalTime;
    simtime_t nextHopWait;
    simtime_t pathDiscoveryTime;

    // state
    unsigned int rreqId = 0; // when sending a new RREQ packet, rreqID incremented by one from the last id used by this node
    uint32_t sequenceNum = 0; // it helps to prevent loops in the routes (RFC 3561 6.1 p11.)
    enum class SearchPhase { RECOVERY_WAIT, SEND_DELAY, READY, WAIT_REPLY };
    struct Search {
        SearchPhase phase = SearchPhase::SEND_DELAY;
        cMessage *timer = nullptr;
        unsigned int ttl = 0;
        unsigned int networkAttempts = 0;
    };
    std::map<L3Address, Search> searches;
    std::deque<L3Address> readySearches;
    simtime_t rateEpoch;
    int64_t rateWindow = 0;
    std::map<RreqIdentifier, simtime_t, RreqIdentifierCompare> rreqsArrivalTime; // maps RREQ id to its arriving time
    using Link = AodvRouteData::Precursor;
    class RerrSend : public cMessage {
      public:
        Ptr<Rerr> header;
        std::set<Link> recipients;
        RerrSend(const Ptr<Rerr>& header, const std::set<Link>& recipients) :
            cMessage("aodv-error-jitter", 100), header(header), recipients(recipients) {}
    };
    std::set<cMessage *> delayedSends;
    using MacLink = std::pair<int, MacAddress>;
    std::map<Link, simtime_t> pendingAcks;
    std::map<Link, simtime_t> blacklist;
    std::map<Link, simtime_t> neighborDeadlines;
    std::set<int> disabledHelloObservation;
    std::map<Link, MacAddress> neighborBindings;
    std::map<MacLink, Link> inverseBindings;
    std::set<MacLink> unusableBindings;
    std::set<int> attributionWarnings;
    unsigned int rerrCount = 0; // num of originated RERR in the last second
    unsigned int rreqCount = 0; // num of originated RREQ in the last second
    std::map<int, simtime_t> lastBroadcastTimes;


    // self messages
    cMessage *helloMsgTimer = nullptr; // timer to send hello messages (only if the feature is enabled)
    cMessage *expungeTimer = nullptr; // timer to clean the routing table out
    cMessage *counterTimer = nullptr; // timer to set rrerCount = rreqCount = 0 in each second
    cMessage *rrepAckTimer = nullptr; // timer to wait for RREP-ACKs (RREP-ACK timeout)
    cMessage *blacklistTimer = nullptr; // timer to clean the blacklist out
    cMessage *neighborTimer = nullptr;
    cMessage *recoveryTimer = nullptr;

    // lifecycle
    bool hasStarted = false;
    bool recovering = false;
    simtime_t recoveryDeadline;
    uint64_t operationalGeneration = 0;

    // internal
    std::map<L3Address, std::deque<Packet *>> delayedPackets; // borrowed from IP
    std::map<L3Address, L3Address> queueOwners; // actual destination -> search destination
    IRoute *defaultGatewayRoute = nullptr;  // Pointer to default gateway route (0.0.0.0/0)

  protected:
    void handleMessageWhenUp(cMessage *msg) override;
    void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /* Route Discovery */
    void startRouteDiscovery(const L3Address& target, unsigned int timeToLive = 0);
    bool hasOngoingRouteDiscovery(const L3Address& destAddr);
    void cancelRouteDiscovery(const L3Address& destAddr);

  private:
    IRoute *findExactRoute(const L3Address& destination) const;
    void expireRouteIfDue(IRoute *route);
    void refreshNeighbor(const L3Address& sender, NetworkInterface *ingress);
    IRoute *selectDataRoute(const L3Address& destination);
    bool prepareDataPacket(Packet *packet, IRoute *route);
    enum class DatagramKind { DATA, CONTROL, REPLY };
    DatagramKind classifyDatagram(const Packet *packet) const;
    void learnBinding(const Link& link, const MacAddress& mac);
    bool resolveFailedLink(const Packet *packet, Link& link);
    void diagnoseAttribution(int interfaceId);
    void scheduleLinkTimer(cMessage *timer, const std::map<Link, simtime_t>& deadlines);
    void failLinkValidation(const Link& link, bool replyFailed);
    void observeIncoming(const Packet *packet);
    void disableHelloObservation(int interfaceId);
    void handleNeighborTimer();
    void handleRecoveryTimer();
    void extendRecovery();
    bool isLocalGatewayDestination(const L3Address& destination) const;
    void sendPreparedControl(Packet *packet);
    bool canSendControl(AodvControlPacketType type) const;
    void configureInterfaces();
    bool isSelectedInterface(const NetworkInterface *candidate) const;
    bool isLocalAodvAddress(const L3Address& address) const;
    bool resolvePreviousHop(const Packet *packet, Link& link) const;
    void refreshSourceRoute(Packet *packet);
    void queueRouteError(const std::vector<UnreachableNode>& nodes, const std::set<Link>& recipients, bool noDelete, simtime_t delay);
    void sendRouteError(RerrSend *message);
    void releaseEligiblePackets();
    void scheduleSearch(const L3Address& target);
    void handleSearchTimer(cMessage *timer);
    void drainReadySearches();
    void refreshRateWindow();
    void eraseSearch(const L3Address& target);
    static simtime_t quantizeAdvertisementLifetime(simtime_t duration);
    simtime_t samplePeriodicJitter();

  protected:
    /* Routing Table management */
    void updateRoutingTable(IRoute *route, const L3Address& nextHop, unsigned int hopCount, bool hasValidDestNum, uint32_t destSeqNum, bool isActive, simtime_t lifeTime, NetworkInterface *nextHopInterface = nullptr);
    IRoute *createRoute(const L3Address& destAddr, const L3Address& nextHop, unsigned int hopCount, bool hasValidDestNum, uint32_t destSeqNum, bool isActive, simtime_t lifeTime, NetworkInterface *nextHopInterface = nullptr);
    bool updateValidRouteLifeTime(const L3Address& destAddr, simtime_t lifetime);
    void scheduleExpungeRoutes();
    void expungeRoutes();

    /* Control packet creators */
    const Ptr<RrepAck> createRREPACK();
    const Ptr<Rrep> createHelloMessage(NetworkInterface *output = nullptr);
    const Ptr<Rreq> createRREQ(const L3Address& destAddr);
    const Ptr<Rrep> createRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& sourceAddr, NetworkInterface *ingress);
    const Ptr<Rrep> createGratuitousRREP(const Ptr<Rreq>& rreq, IRoute *originatorRoute);
    const Ptr<Rerr> createRERR(const std::vector<UnreachableNode>& unreachableNodes);

    /* Control Packet handlers */
    void handleRREP(const Ptr<Rrep>& rrep, const L3Address& sourceAddr, NetworkInterface *ingress);
    void handleRREQ(const Ptr<Rreq>& rreq, const L3Address& sourceAddr, unsigned int timeToLive, NetworkInterface *ingress);
    void handleRERR(const Ptr<const Rerr>& rerr, const L3Address& sourceAddr, NetworkInterface *ingress);
    void handleHelloMessage(const Ptr<Rrep>& helloMessage, NetworkInterface *ingress);
    void handleRREPACK(const Ptr<const RrepAck>& rrepACK, const L3Address& neighborAddr, NetworkInterface *ingress);

    /* Control Packet sender methods */
    void sendRREPACK(const Ptr<RrepAck>& rrepACK, const L3Address& destAddr, NetworkInterface *output);
    void sendRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive);
    void sendGRREP(const Ptr<Rrep>& grrep, const L3Address& destAddr, unsigned int timeToLive);

    /* Control Packet forwarders */
    void forwardRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive, NetworkInterface *output);
    void forwardRREQ(const Ptr<Rreq>& rreq, unsigned int timeToLive);

    /* Self message handlers */
    void handleRREPACKTimer();
    void handleBlackListTimer();
    void sendHelloMessagesIfNeeded();

    /* General functions to handle route errors */
    void sendRERRWhenNoRouteToForward(Packet *packet);
    void handleLinkBreakSendRERR(const Link& link);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    /* Netfilter hooks */
    Result ensureRouteForDatagram(Packet *datagram, bool local);
    virtual Result datagramPreRoutingHook(Packet *datagram) override { Enter_Method("datagramPreRoutingHook"); return ensureRouteForDatagram(datagram, false); }
    virtual Result datagramForwardHook(Packet *datagram) override;
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram) override;
    virtual Result datagramLocalOutHook(Packet *datagram) override { Enter_Method("datagramLocalOutHook"); return ensureRouteForDatagram(datagram, true); }
    void delayDatagram(Packet *datagram);

    /* Helper functions */
    L3Address getSelfIPAddress() const;
    void sendAODVPacket(const Ptr<AodvControlPacket>& packet, const L3Address& destAddr, unsigned int timeToLive, double delay, NetworkInterface *output = nullptr);
    void processPacket(Packet *pk);
    void clearState(bool disposeQueued = true);
    void checkIpVersionAndPacketTypeCompatibility(AodvControlPacketType packetType);
    bool isExternalAddress(const L3Address& destAddr) const;
    void createDefaultGatewayRoute();
    void updateGatewayRoute();

    /* UDP callback interface */
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    /* Lifecycle */
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    Aodv();
    virtual ~Aodv();
};

} // namespace aodv
} // namespace inet

#endif

