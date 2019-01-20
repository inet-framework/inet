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
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/modes/PimBase.h"
#include "inet/routing/pim/tables/PimInterfaceTable.h"
#include "inet/routing/pim/tables/PimNeighborTable.h"

namespace inet {

/**
 * Implementation of PIM-DM protocol (RFC 3973).
 *
 *
 */
class INET_API PimDm : public PimBase, protected cListener
{
  private:
    struct Route;

    struct UpstreamInterface : public Interface
    {
        enum Flags {
            SOURCE_DIRECTLY_CONNECTED = 0x01
        };

        enum GraftPruneState {
            FORWARDING,  // oiflist != nullptr
            PRUNED,      // olist is empty
            ACK_PENDING  // waiting for a Graft Ack
        };

        enum OriginatorState { NOT_ORIGINATOR, ORIGINATOR };

        Ipv4Address nextHop;    // rpf neighbor

        // graft prune state
        GraftPruneState graftPruneState;
        cMessage *graftRetryTimer;    // scheduled in ACK_PENDING state for sending the next Graft message
        cMessage *overrideTimer;      // when expires we are overriding a prune
        simtime_t lastPruneSentTime;  // for rate limiting prune messages, 0 if no prune was sent

        // originator state
        OriginatorState originatorState;
        cMessage *sourceActiveTimer;    // when expires we are going back to NOT_ORIGINATOR state
        cMessage *stateRefreshTimer;    // scheduled in ORIGINATOR state for sending the next StateRefresh message
        unsigned short maxTtlSeen;

        UpstreamInterface(Route *owner, InterfaceEntry *ie, Ipv4Address neighbor, bool isSourceDirectlyConnected)
            : Interface(owner, ie), nextHop(neighbor),
            graftPruneState(FORWARDING), graftRetryTimer(nullptr), overrideTimer(nullptr), lastPruneSentTime(0.0),
            originatorState(NOT_ORIGINATOR), sourceActiveTimer(nullptr), stateRefreshTimer(nullptr), maxTtlSeen(0)
        { setFlag(SOURCE_DIRECTLY_CONNECTED, isSourceDirectlyConnected); }
        virtual ~UpstreamInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PimDm *pimdm() const { return check_and_cast<PimDm *>(owner->owner); }
        int getInterfaceId() const { return ie->getInterfaceId(); }
        Ipv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
        GraftPruneState getGraftPruneState() const { return graftPruneState; }
        cMessage * getGraftRetryTimer() const { return graftRetryTimer; }
        cMessage * getOverrideTimer() const { return overrideTimer; }
        simtime_t getLastPruneSentTime() const { return lastPruneSentTime; }
        bool isSourceDirectlyConnected() const { return isFlagSet(SOURCE_DIRECTLY_CONNECTED); }
        OriginatorState getOriginatorState() const { return originatorState; }
        cMessage * getSourceActiveTimer() const { return sourceActiveTimer; }
        cMessage * getStateRefreshTimer() const { return stateRefreshTimer; }
        unsigned short getMaxTtlSeen() const { return maxTtlSeen; }

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
            NO_INFO,          // no prune info, neither pruneTimer or prunePendingTimer is running
            PRUNE_PENDING,    // received a prune from a downstream neighbor, waiting for an override
            PRUNED            // received a prune from a downstream neighbor and it was not overridden
        };

        // prune state
        PruneState pruneState;
        cMessage *pruneTimer;         // scheduled when entering into PRUNED state, when expires the interface goes to NO_INFO (forwarding) state
        cMessage *prunePendingTimer;  // scheduled when a Prune is received, when expires the interface goes to PRUNED state

        DownstreamInterface(Route *owner, InterfaceEntry *ie)
            : Interface(owner, ie),
            pruneState(NO_INFO), pruneTimer(nullptr), prunePendingTimer(nullptr)
        { ASSERT(owner), ASSERT(ie); }
        ~DownstreamInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PimDm *pimdm() const { return check_and_cast<PimDm *>(owner->owner); }
        PruneState getPruneState() { return pruneState; }
        cMessage * getPruneTimer() const { return pruneTimer; }
        cMessage * getPrunePendingTimer() const { return prunePendingTimer; }
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

        Route(PimDm *owner, Ipv4Address source, Ipv4Address group)
            : RouteEntry(owner, source, group), upstreamInterface(nullptr) {}
        virtual ~Route();
        DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId) const;
        DownstreamInterface *createDownstreamInterface(InterfaceEntry *ie);
        DownstreamInterface *removeDownstreamInterface(int interfaceId);
        bool isOilistNull();
    };

    friend std::ostream& operator<<(std::ostream& out, const PimDm::Route& sourceGroup);

    typedef std::map<SourceAndGroup, Route *> RoutingTable;

    // for updating the forwarding state of the route when the state of the downstream interface changes
    class PimDmOutInterface : public IMulticastRoute::OutInterface
    {
        DownstreamInterface *downstream;

      public:
        PimDmOutInterface(InterfaceEntry *ie, DownstreamInterface *downstream)
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
    PimDm() : PimBase(PimInterface::DenseMode) {}
    virtual ~PimDm();

    const static std::string graftPruneStateString(UpstreamInterface::GraftPruneState ps);
    const static std::string originatorStateString(UpstreamInterface::OriginatorState os);
    const static std::string pruneStateString(DownstreamInterface::PruneState ps);

  private:
    // process signals
    void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    void unroutableMulticastPacketArrived(Ipv4Address srcAddress, Ipv4Address destAddress, unsigned short ttl);
    void multicastPacketArrivedOnNonRpfInterface(Ipv4Address group, Ipv4Address source, int interfaceId);
    void multicastPacketArrivedOnRpfInterface(int interfaceId, Ipv4Address group, Ipv4Address source, unsigned short ttl);
    void multicastReceiverAdded(InterfaceEntry *ie, Ipv4Address newAddr);
    void multicastReceiverRemoved(InterfaceEntry *ie, Ipv4Address oldAddr);
    void rpfInterfaceHasChanged(Ipv4MulticastRoute *route, Ipv4Route *routeToSource);

    // process timers
    void processPruneTimer(cMessage *timer);
    void processPrunePendingTimer(cMessage *timer);
    void processGraftRetryTimer(cMessage *timer);
    void processOverrideTimer(cMessage *timer);
    void processSourceActiveTimer(cMessage *timer);
    void processStateRefreshTimer(cMessage *timer);
    void processAssertTimer(cMessage *timer);

    // process PIM packets
    void processJoinPrunePacket(Packet *pk);
    void processGraftPacket(Packet *pk);
    void processGraftAckPacket(Packet *pk);
    void processStateRefreshPacket(Packet *pk);
    void processAssertPacket(Packet *pk);

    void processPrune(Route *route, int intId, int holdTime, int numRpfNeighbors, Ipv4Address upstreamNeighborField);
    void processJoin(Route *route, int intId, int numRpfNeighbors, Ipv4Address upstreamNeighborField);
    void processGraft(Ipv4Address source, Ipv4Address group, Ipv4Address sender, int intId);
    void processAssert(Interface *downstream, AssertMetric receivedMetric, int stateRefreshInterval);

    // process olist changes
    void processOlistEmptyEvent(Route *route);
    void processOlistNonEmptyEvent(Route *route);

    // create and send PIM packets
    void sendPrunePacket(Ipv4Address nextHop, Ipv4Address src, Ipv4Address grp, int holdTime, int intId);
    void sendJoinPacket(Ipv4Address nextHop, Ipv4Address source, Ipv4Address group, int interfaceId);
    void sendGraftPacket(Ipv4Address nextHop, Ipv4Address src, Ipv4Address grp, int intId);
    void sendGraftAckPacket(Packet *pk, const Ptr<const PimGraft>& graftPacket);
    void sendStateRefreshPacket(Ipv4Address originator, Route *route, DownstreamInterface *downstream, unsigned short ttl);
    void sendAssertPacket(Ipv4Address source, Ipv4Address group, AssertMetric metric, InterfaceEntry *ie);
    void sendToIP(Packet *packet, Ipv4Address source, Ipv4Address dest, int outInterfaceId);

    // helpers
    void restartTimer(cMessage *timer, double interval);
    void cancelAndDeleteTimer(cMessage *& timer);
    PimInterface *getIncomingInterface(InterfaceEntry *fromIE);
    Ipv4MulticastRoute *findIpv4MulticastRoute(Ipv4Address group, Ipv4Address source);
    Route *findRoute(Ipv4Address source, Ipv4Address group);
    void deleteRoute(Ipv4Address source, Ipv4Address group);
    void clearRoutes();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void initialize(int stage) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void stopPIMRouting();
};

}    // namespace inet

#endif // ifndef __INET_PIMDM_H

