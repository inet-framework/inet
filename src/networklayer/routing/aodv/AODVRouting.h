//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef AODVROUTING_H_
#define AODVROUTING_H_

#include "INETDefs.h"
#include "IInterfaceTable.h"
//#include "IAddressType.h"
#include "IPv4Address.h"
#include "IRoutingTable.h"
#include "INetfilter.h"
#include "ILifecycle.h"
#include "NodeStatus.h"
#include "NotificationBoard.h"
#include "UDPSocket.h"
#include "AODVRouteData.h"
#include "UDPPacket.h"
#include "AODVControlPackets_m.h"
#include <map>

/*
 * This class implements AODV routing protocol and Netfilter hooks
 * in the IP-layer required by this protocol.
 */

class INET_API AODVRouting : public cSimpleModule, public ILifecycle, public INetfilter::IHook, public INotifiable
{
  protected:
    /*
     * It implements a unique identifier for an arbitrary RREQ message
     * in the network. See: rreqsArrivalTime.
     */
    class RREQIdentifier
    {
      public:
        IPv4Address originatorAddr;
        unsigned int rreqID;
        RREQIdentifier(const IPv4Address& originatorAddr, unsigned int rreqID) : originatorAddr(originatorAddr), rreqID(rreqID) {};
        bool operator==(const RREQIdentifier& other) const
        {
            return this->originatorAddr == other.originatorAddr && this->rreqID == other.rreqID;
        }
    };

    class RREQIdentifierCompare
    {
      public:
        bool operator()(const RREQIdentifier& lhs, const RREQIdentifier& rhs) const
        {
            return lhs.rreqID < rhs.rreqID;
        }
    };

    // context
    //IAddressType *addressType;    // to support both IPv4 and v6 addresses.

    // environment
    cModule *host;
    IRoutingTable *routingTable;
    IInterfaceTable *interfaceTable;
    INetfilter *networkProtocol;
    NotificationBoard *nb;

    // AODV parameters: the following parameters are configurable, see the NED file for more info.
    unsigned int rerrRatelimit;
    unsigned int aodvUDPPort;
    bool askGratuitousRREP;
    bool useHelloMessages;
    simtime_t maxJitter;
    simtime_t activeRouteTimeout;
    simtime_t helloInterval;
    unsigned int netDiameter;
    unsigned int rreqRetries;
    unsigned int rreqRatelimit;
    unsigned int timeoutBuffer;
    unsigned int ttlStart;
    unsigned int ttlIncrement;
    unsigned int ttlThreshold;
    unsigned int localAddTTL;
    unsigned int allowedHelloLoss;
    simtime_t nodeTraversalTime;
    cPar *jitterPar;
    cPar *periodicJitter;

    // the following parameters are calculated from the parameters defined above
    // see the NED file for more info
    simtime_t deletePeriod;
    simtime_t myRouteTimeout;
    simtime_t blacklistTimeout;
    simtime_t netTraversalTime;
    simtime_t nextHopWait;
    simtime_t pathDiscoveryTime;

    // state
    unsigned int rreqId;    // when sending a new RREQ packet, rreqID incremented by one from the last id used by this node
    unsigned int sequenceNum;    // it helps to prevent loops in the routes (RFC 3561 6.1 p11.)
    std::map<IPv4Address, WaitForRREP *> waitForRREPTimers;    // timeout for Route Replies
    std::map<RREQIdentifier, simtime_t, RREQIdentifierCompare> rreqsArrivalTime;    // maps RREQ id to its arriving time
    IPv4Address failedNextHop;    // next hop to the destination who failed to send us RREP-ACK
    std::map<IPv4Address, simtime_t> blacklist;    // we don't accept RREQs from blacklisted nodes
    unsigned int rerrCount;    // num of originated RERR in the last second
    unsigned int rreqCount;    // num of originated RREQ in the last second
    simtime_t lastBroadcastTime;    // the last time when any control packet was broadcasted
    std::map<IPv4Address, unsigned int> addressToRreqRetries; // number of re-discovery attempts per address

    // self messages
    cMessage *helloMsgTimer;    // timer to send hello messages (only if the feature is enabled)
    cMessage *expungeTimer;    // timer to clean the routing table out
    cMessage *counterTimer;    // timer to set rrerCount = rreqCount = 0 in each second
    cMessage *rrepAckTimer;    // timer to wait for RREP-ACKs (RREP-ACK timeout)
    cMessage *blacklistTimer;    // timer to clean the blacklist out

    // lifecycle
    simtime_t rebootTime;    // the last time when the node rebooted
    bool isOperational;

    // internal
    std::multimap<IPv4Address, IPv4Datagram *> targetAddressToDelayedPackets;    // queue for the datagrams we have no route for

  protected:
    void handleMessage(cMessage *msg);
    void initialize(int stage);
    virtual int numInitStages() const { return 5; }

    /* Route Discovery */
    void startRouteDiscovery(const IPv4Address& target, unsigned int timeToLive = 0);
    void completeRouteDiscovery(const IPv4Address& target);
    bool hasOngoingRouteDiscovery(const IPv4Address& destAddr);
    void cancelRouteDiscovery(const IPv4Address& destAddr);

    /* Routing Table management */
    void updateRoutingTable(IPv4Route *route, const IPv4Address& nextHop, unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum, bool isActive, simtime_t lifeTime);
    IPv4Route *createRoute(const IPv4Address& destAddr, const IPv4Address& nextHop, unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum, bool isActive, simtime_t lifeTime);
    bool updateValidRouteLifeTime(const IPv4Address& destAddr, simtime_t lifetime);
    void scheduleExpungeRoutes();
    void expungeRoutes();

    /* Control packet creators */
    AODVRREPACK *createRREPACK();
    AODVRREP *createHelloMessage();
    AODVRREQ *createRREQ(const IPv4Address& destAddr);
    AODVRREP *createRREP(AODVRREQ *rreq, IPv4Route *destRoute, IPv4Route *originatorRoute, const IPv4Address& sourceAddr);
    AODVRREP *createGratuitousRREP(AODVRREQ *rreq, IPv4Route *originatorRoute);
    AODVRERR *createRERR(const std::vector<UnreachableNode>& unreachableNodes);

    /* Control Packet handlers */
    void handleRREP(AODVRREP *rrep, const IPv4Address& sourceAddr);
    void handleRREQ(AODVRREQ *rreq, const IPv4Address& sourceAddr, unsigned int timeToLive);
    void handleRERR(AODVRERR *rerr, const IPv4Address& sourceAddr);
    void handleHelloMessage(AODVRREP *helloMessage);
    void handleRREPACK(AODVRREPACK *rrepACK, const IPv4Address& neighborAddr);

    /* Control Packet sender methods */
    void sendRREQ(AODVRREQ *rreq, const IPv4Address& destAddr, unsigned int timeToLive);
    void sendRREPACK(AODVRREPACK *rrepACK, const IPv4Address& destAddr);
    void sendRREP(AODVRREP *rrep, const IPv4Address& destAddr, unsigned int timeToLive);
    void sendGRREP(AODVRREP *grrep, const IPv4Address& destAddr, unsigned int timeToLive);

    /* Control Packet forwarders */
    void forwardRREP(AODVRREP *rrep, const IPv4Address& destAddr, unsigned int timeToLive);
    void forwardRREQ(AODVRREQ *rreq, unsigned int timeToLive);

    /* Self message handlers */
    void handleRREPACKTimer();
    void handleBlackListTimer();
    void sendHelloMessagesIfNeeded();
    void handleWaitForRREP(WaitForRREP *rrepTimer);

    /* General functions to handle route errors */
    void sendRERRWhenNoRouteToForward(const IPv4Address& unreachableAddr);
    void handleLinkBreakSendRERR(const IPv4Address& unreachableAddr);
    virtual void receiveChangeNotification(int signalID, const cObject *obj);

    /* Netfilter hooks */
    Result ensureRouteForDatagram(IPv4Datagram *datagram);
    virtual Result datagramPreRoutingHook(IPv4Datagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address& nextHopAddress) { Enter_Method("datagramPreRoutingHook"); return ensureRouteForDatagram(datagram); }
    virtual Result datagramForwardHook(IPv4Datagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address& nextHopAddress);
    virtual Result datagramPostRoutingHook(IPv4Datagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address& nextHopAddress) { return ACCEPT; }
    virtual Result datagramLocalInHook(IPv4Datagram *datagram, const InterfaceEntry *inputInterfaceEntry) { return ACCEPT; }
    virtual Result datagramLocalOutHook(IPv4Datagram *datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address& nextHopAddress) { Enter_Method("datagramLocalOutHook"); return ensureRouteForDatagram(datagram); }
    void delayDatagram(IPv4Datagram *datagram);

    /* Helper functions */
    IPv4Address getSelfIPAddress() const;
    void sendAODVPacket(AODVControlPacket *packet, const IPv4Address& destAddr, unsigned int timeToLive, double delay);
    void clearState();

    /* Lifecycle */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  public:
    AODVRouting();
    virtual ~AODVRouting();
};

#endif // ifndef AODVROUTING_H_

