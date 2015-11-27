//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMDM_H
#define __INET_PIMDM_H

#include "inet/common/INETDefs.h"

#include "inet/routing/pim/PIMPacket_m.h"
#include "inet/common/NotifierConsts.h"
#include "inet/routing/pim/tables/PIMNeighborTable.h"
#include "inet/routing/pim/tables/PIMInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4Route.h"
#include "inet/routing/pim/modes/PIMBase.h"

namespace inet {

/**
 * Implementation of PIM-DM protocol (RFC 3973).
 *
 *
 */
class INET_API PIMDM : public PIMBase, protected cListener
{
  private:
    // per (S,G) state
    struct Route;

    struct UpstreamInterface : public Interface
    {
        enum Flags {
            SOURCE_DIRECTLY_CONNECTED = 0x01
        };

        enum GraftPruneState {
            FORWARDING,    // oiflist != nullptr
            PRUNED,    // olist is empty
            ACK_PENDING    // waiting for a Graft Ack
        };

        enum OriginatorState { NOT_ORIGINATOR, ORIGINATOR };

        IPv4Address nextHop;    // rpf neighbor

        // graft prune state
        GraftPruneState graftPruneState;
        cMessage *graftRetryTimer;    // scheduled in ACK_PENDING state for sending the next Graft message
        cMessage *overrideTimer;    // when expires we are overriding a prune
        simtime_t lastPruneSentTime;    // for rate limiting prune messages, 0 if no prune was sent

        // originator state
        OriginatorState originatorState;
        cMessage *sourceActiveTimer;    // when expires we are going back to NOT_ORIGINATOR state
        cMessage *stateRefreshTimer;    // scheduled in ORIGINATOR state for sending the next StateRefresh message
        unsigned short maxTtlSeen;

        UpstreamInterface(Route *owner, InterfaceEntry *ie, IPv4Address neighbor, bool isSourceDirectlyConnected)
            : Interface(owner, ie), nextHop(neighbor),
            graftPruneState(FORWARDING), graftRetryTimer(nullptr), overrideTimer(nullptr), lastPruneSentTime(0.0),
            originatorState(NOT_ORIGINATOR), sourceActiveTimer(nullptr), stateRefreshTimer(nullptr), maxTtlSeen(0)
        { ASSERT(owner); ASSERT(ie); setFlag(SOURCE_DIRECTLY_CONNECTED, isSourceDirectlyConnected); }
        virtual ~UpstreamInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PIMDM *pimdm() const { return check_and_cast<PIMDM *>(owner->owner); }
        int getInterfaceId() const { return ie->getInterfaceId(); }
        IPv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
        bool isSourceDirectlyConnected() const { return isFlagSet(SOURCE_DIRECTLY_CONNECTED); }

        void startGraftRetryTimer();
        void startOverrideTimer();
        void startSourceActiveTimer();
        void startStateRefreshTimer();
        void startPruneLimitTimer() { lastPruneSentTime = simTime(); }
        void stopPruneLimitTimer() { lastPruneSentTime = 0; }
        bool isPruneLimitTimerRunning() { return lastPruneSentTime > 0.0 && simTime() < lastPruneSentTime + pimdm()->pruneLimitInterval; }
    };

    struct DownstreamInterface : public Interface
    {
        enum Flags {
            HAS_CONNECTED_RECEIVERS = 0x01
        };

        enum PruneState {
            NO_INFO,    // no prune info, neither pruneTimer or prunePendingTimer is running
            PRUNE_PENDING,    // received a prune from a downstream neighbor, waiting for an override
            PRUNED    // received a prune from a downstream neighbor and it was not overridden
        };

        // prune state
        PruneState pruneState;
        cMessage *pruneTimer;    // scheduled when entering into PRUNED state, when expires the interface goes to NO_INFO (forwarding) state
        cMessage *prunePendingTimer;    // scheduled when a Prune is received, when expires the interface goes to PRUNED state

        DownstreamInterface(Route *owner, InterfaceEntry *ie)
            : Interface(owner, ie),
            pruneState(NO_INFO), pruneTimer(nullptr), prunePendingTimer(nullptr)
        { ASSERT(owner), ASSERT(ie); }
        ~DownstreamInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PIMDM *pimdm() const { return check_and_cast<PIMDM *>(owner->owner); }
        bool hasConnectedReceivers() const { return isFlagSet(HAS_CONNECTED_RECEIVERS); }
        void setHasConnectedReceivers(bool value) { setFlag(HAS_CONNECTED_RECEIVERS, value); }
        bool isInOlist() const;
        void startPruneTimer(double holdTime);
        void stopPruneTimer();
        void startPrunePendingTimer(double overrideInterval);
        void stopPrunePendingTimer();
    };

    struct Route : public RouteEntry
    {
        UpstreamInterface *upstreamInterface;
        std::vector<DownstreamInterface *> downstreamInterfaces;

        Route(PIMDM *owner, IPv4Address source, IPv4Address group)
            : RouteEntry(owner, source, group), upstreamInterface(nullptr) {}
        virtual ~Route();
        DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId) const;
        DownstreamInterface *createDownstreamInterface(InterfaceEntry *ie);
        DownstreamInterface *removeDownstreamInterface(int interfaceId);
        bool isOilistNull();
    };

    friend std::ostream& operator<<(std::ostream& out, const Route *route);

    typedef std::map<SourceAndGroup, Route *> RoutingTable;

    // for updating the forwarding state of the route when the state of the downstream interface changes
    class PIMDMOutInterface : public IMulticastRoute::OutInterface
    {
        DownstreamInterface *downstream;

      public:
        PIMDMOutInterface(InterfaceEntry *ie, DownstreamInterface *downstream)
            : IMulticastRoute::OutInterface(ie), downstream(downstream) {}
        virtual bool isEnabled() override { return downstream->isInOlist(); }
    };

  private:
    // parameters
    double pruneInterval = 0;
    double pruneLimitInterval = 0;
    double overrideInterval = 0;
    double propagationDelay = 0;
    double graftRetryInterval = 0;
    double sourceActiveInterval = 0;
    double stateRefreshInterval = 0;
    double assertTime = 0;

    // signals
    static simsignal_t sentGraftPkSignal;
    static simsignal_t rcvdGraftPkSignal;
    static simsignal_t sentGraftAckPkSignal;
    static simsignal_t rcvdGraftAckPkSignal;
    static simsignal_t sentJoinPrunePkSignal;
    static simsignal_t rcvdJoinPrunePkSignal;
    static simsignal_t sentAssertPkSignal;
    static simsignal_t rcvdAssertPkSignal;
    static simsignal_t sentStateRefreshPkSignal;
    static simsignal_t rcvdStateRefreshPkSignal;

    // state
    RoutingTable routes;

  public:
    PIMDM() : PIMBase(PIMInterface::DenseMode) {}
    virtual ~PIMDM();

  private:
    // process signals
    void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
    void unroutableMulticastPacketArrived(IPv4Address srcAddress, IPv4Address destAddress, unsigned short ttl);
    void multicastPacketArrivedOnNonRpfInterface(IPv4Address group, IPv4Address source, int interfaceId);
    void multicastPacketArrivedOnRpfInterface(int interfaceId, IPv4Address group, IPv4Address source, unsigned short ttl);
    void multicastReceiverAdded(InterfaceEntry *ie, IPv4Address newAddr);
    void multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address oldAddr);
    void rpfInterfaceHasChanged(IPv4MulticastRoute *route, IPv4Route *routeToSource);

    // process timers
    void processPruneTimer(cMessage *timer);
    void processPrunePendingTimer(cMessage *timer);
    void processGraftRetryTimer(cMessage *timer);
    void processOverrideTimer(cMessage *timer);
    void processSourceActiveTimer(cMessage *timer);
    void processStateRefreshTimer(cMessage *timer);
    void processAssertTimer(cMessage *timer);

    // process PIM packets
    void processJoinPrunePacket(PIMJoinPrune *pkt);
    void processGraftPacket(PIMGraft *pkt);
    void processGraftAckPacket(PIMGraftAck *pkt);
    void processStateRefreshPacket(PIMStateRefresh *pkt);
    void processAssertPacket(PIMAssert *pkt);

    void processPrune(Route *route, int intId, int holdTime, int numRpfNeighbors, IPv4Address upstreamNeighborField);
    void processJoin(Route *route, int intId, int numRpfNeighbors, IPv4Address upstreamNeighborField);
    void processGraft(IPv4Address source, IPv4Address group, IPv4Address sender, int intId);
    void processAssert(Interface *downstream, AssertMetric receivedMetric, int stateRefreshInterval);

    // process olist changes
    void processOlistEmptyEvent(Route *route);
    void processOlistNonEmptyEvent(Route *route);

    // create and send PIM packets
    void sendPrunePacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int holdTime, int intId);
    void sendJoinPacket(IPv4Address nextHop, IPv4Address source, IPv4Address group, int interfaceId);
    void sendGraftPacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
    void sendGraftAckPacket(PIMGraft *msg);
    void sendStateRefreshPacket(IPv4Address originator, Route *route, DownstreamInterface *downstream, unsigned short ttl);
    void sendAssertPacket(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie);
    void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId);

    // helpers
    void restartTimer(cMessage *timer, double interval);
    void cancelAndDeleteTimer(cMessage *& timer);
    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
    IPv4MulticastRoute *findIPv4MulticastRoute(IPv4Address group, IPv4Address source);
    Route *findRoute(IPv4Address source, IPv4Address group);
    void deleteRoute(IPv4Address source, IPv4Address group);
    void clearRoutes();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void initialize(int stage) override;
    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;
    virtual void stopPIMRouting();
};

}    // namespace inet

#endif // ifndef __INET_PIMDM_H

