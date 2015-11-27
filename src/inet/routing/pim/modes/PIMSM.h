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
// Authors: Veronika Rybova, Tomas Prochazka (xproch21@stud.fit.vutbr.cz),
//          Vladimir Vesely (ivesely@fit.vutbr.cz), Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMSM_H
#define __INET_PIMSM_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ipv4/IPv4Route.h"
#include "inet/routing/pim/modes/PIMBase.h"

namespace inet {

#define KAT        180.0                       /**< Keep alive timer, if RPT is disconnect */
#define MAX_TTL    255                     /**< Maximum TTL */

/**
 * Implementation of PIM-SM protocol (RFC 4601).
 *
 * Protocol state is stored in two tables:
 * - gRoutes table contains (*,G) state
 * - sgRoutes table contains (S,G) state
 * Note that (*,*,RP) and (S,G,rpt) state is currently missing.
 *
 * The routes stored in the tables are not the same as the routes
 * of the IPv4RoutingTable. The RFC defines the terms "Tree Information Base" (TIB)
 * and "Multicast Forwaring Information Base" (MFIB). According to this division,
 * TIB is the state of this module, while MFIB is stored by IPv4RoutingTable.
 *
 * Incoming packets, notifications, and timer events may cause a change
 * of the TIB, and of the MFIB (if the forwarding rules change).
 *
 *
 */
class INET_API PIMSM : public PIMBase, protected cListener
{
  private:
    struct Route;
    friend std::ostream& operator<<(std::ostream& out, const SourceAndGroup& sourceGroup);
    friend std::ostream& operator<<(std::ostream& out, const Route& sourceGroup);

    struct PimsmInterface : public Interface
    {
        cMessage *expiryTimer;

        enum Flags {
            RECEIVER_INCLUDE = 1 << 0,    // local_receiver_include(S,G,I) or local_receiver_include(*,G,I)
            RECEIVER_EXCLUDE = 1 << 1,    // local_receiver_exclude(S,G,I)
            COULD_ASSERT = 1 << 2,    // CouldAssert(S,G,I)
            ASSERT_TRACKING_DESIRED = 1 << 3    // AssertTrackingDesired(S,G,I)
        };

        PimsmInterface(Route *owner, InterfaceEntry *ie);
        virtual ~PimsmInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PIMSM *pimsm() const { return check_and_cast<PIMSM *>(owner->owner); }
        void startExpiryTimer(double holdTime);

        bool localReceiverInclude() const { return isFlagSet(RECEIVER_INCLUDE); }
        void setLocalReceiverInclude(bool value) { setFlag(RECEIVER_INCLUDE, value); }
        bool localReceiverExclude() const { return isFlagSet(RECEIVER_EXCLUDE); }
        void setLocalReceiverExclude(bool value) { setFlag(RECEIVER_EXCLUDE, value); }
        bool couldAssert() const { return isFlagSet(COULD_ASSERT); }
        void setCouldAssert(bool value) { setFlag(COULD_ASSERT, value); }
        bool assertTrackingDesired() const { return isFlagSet(ASSERT_TRACKING_DESIRED); }
        void setAssertTrackingDesired(bool value) { setFlag(ASSERT_TRACKING_DESIRED, value); }

        bool pimInclude() const
        {
            return localReceiverInclude() &&
                   ((    /*I_am_DR AND*/ assertState != I_LOST_ASSERT) || assertState == I_WON_ASSERT);
        }

        bool pimExclude() const
        {
            return localReceiverExclude() &&
                   ((    /*I_am_DR AND*/ assertState != I_LOST_ASSERT) || assertState == I_WON_ASSERT);
        }
    };

    // upstream interface is toward the RP or toward the source
    struct UpstreamInterface : public PimsmInterface
    {
        IPv4Address nextHop;    // RPF nexthop, <unspec> at the DR in (S,G) routes

        UpstreamInterface(Route *owner, InterfaceEntry *ie, IPv4Address nextHop)
            : PimsmInterface(owner, ie), nextHop(nextHop) {}
        int getInterfaceId() const { return ie->getInterfaceId(); }
        IPv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
    };

    struct DownstreamInterface : public PimsmInterface
    {
        /** States of each outgoing interface. */
        enum JoinPruneState { NO_INFO, JOIN, PRUNE_PENDING };

        JoinPruneState joinPruneState;
        cMessage *prunePendingTimer;

        DownstreamInterface(Route *owner, InterfaceEntry *ie, JoinPruneState joinPruneState, bool show = true)
            : PimsmInterface(owner, ie), joinPruneState(joinPruneState), prunePendingTimer(nullptr) {}
        virtual ~DownstreamInterface();

        int getInterfaceId() const { return ie->getInterfaceId(); }
        bool isInImmediateOlist() const;
        bool isInInheritedOlist() const;
        void startPrunePendingTimer(double joinPruneOverrideInterval);
    };

    typedef std::vector<DownstreamInterface *> DownstreamInterfaceVector;

    class PIMSMOutInterface : public IMulticastRoute::OutInterface
    {
        DownstreamInterface *downstream;

      public:
        PIMSMOutInterface(DownstreamInterface *downstream)
            : OutInterface(downstream->ie), downstream(downstream) {}
        virtual bool isEnabled() override { return downstream->isInInheritedOlist(); }
    };

    enum RouteType {
        RP,    // (*,*,RP)
        G,    // (*,G)
        SG,    // (S,G)
        SGrpt    // (S,G,rpt)
    };

    // Holds (*,G), (S,G) or (S,G,rpt) state
    struct Route : public RouteEntry
    {
        enum Flags {
            PRUNED = 0x01,    // UpstreamJPState
            REGISTER = 0x02,    // Register flag
            SPT_BIT = 0x04,    // used to distinguish whether to forward on (*,*,RP)/(*,G) or on (S,G) state
            JOIN_DESIRED = 0x08,
            SOURCE_DIRECTLY_CONNECTED = 0x10
        };

        RouteType type;
        IPv4Address rpAddr;

        // related routes
        Route *rpRoute;
        Route *gRoute;
        Route *sgrptRoute;

        //Originated from destination.Ensures loop freeness.
        unsigned int sequencenumber;
        //Time of routing table entry creation
        simtime_t installtime;    // XXX not used

        cMessage *keepAliveTimer;    // only for (S,G) routes
        cMessage *joinTimer;

        // Register state (only for (S,G) at the DR)
        enum RegisterState { RS_NO_INFO, RS_JOIN, RS_PRUNE, RS_JOIN_PENDING };
        RegisterState registerState;
        cMessage *registerStopTimer;

        // interface specific state
        UpstreamInterface *upstreamInterface;    // may be nullptr at RP and at DR
        DownstreamInterfaceVector downstreamInterfaces;    ///< Out interfaces (downstream)

      public:
        Route(PIMSM *owner, RouteType type, IPv4Address origin, IPv4Address group);
        virtual ~Route();
        PIMSM *pimsm() const { return check_and_cast<PIMSM *>(owner); }

        void addDownstreamInterface(DownstreamInterface *outInterface);
        void removeDownstreamInterface(unsigned int i);

        DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId);
        DownstreamInterface *getDownstreamInterfaceByInterfaceId(int interfaceId);
        int findDownstreamInterface(InterfaceEntry *ie);

        bool isImmediateOlistNull();
        bool isInheritedOlistNull();
        bool joinDesired() const { return isFlagSet(JOIN_DESIRED); }
        bool isSourceDirectlyConnected() const { return isFlagSet(SOURCE_DIRECTLY_CONNECTED); }

        void startKeepAliveTimer(double keepAlivePeriod);
        void startRegisterStopTimer(double interval);
        void startJoinTimer(double joinPrunePeriod);
    };

    typedef std::map<SourceAndGroup, Route *> RoutingTable;

    // parameters
    IPv4Address rpAddr;
    double joinPrunePeriod = 0;
    double defaultOverrideInterval = 0;
    double defaultPropagationDelay = 0;
    double keepAlivePeriod = 0;
    double rpKeepAlivePeriod = 0;
    double registerSuppressionTime = 0;
    double registerProbeTime = 0;
    double assertTime = 0;
    double assertOverrideInterval = 0;

    // signals
    static simsignal_t sentRegisterPkSignal;
    static simsignal_t rcvdRegisterPkSignal;
    static simsignal_t sentRegisterStopPkSignal;
    static simsignal_t rcvdRegisterStopPkSignal;
    static simsignal_t sentJoinPrunePkSignal;
    static simsignal_t rcvdJoinPrunePkSignal;
    static simsignal_t sentAssertPkSignal;
    static simsignal_t rcvdAssertPkSignal;

    // state
    RoutingTable gRoutes;
    RoutingTable sgRoutes;

  public:
    PIMSM() : PIMBase(PIMInterface::SparseMode) {}
    virtual ~PIMSM();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;
    virtual void stopPIMRouting();
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

  private:
    // process PIM messages
    void processJoinPrunePacket(PIMJoinPrune *pkt);
    void processRegisterPacket(PIMRegister *pkt);
    void processRegisterStopPacket(PIMRegisterStop *pkt);
    void processAssertPacket(PIMAssert *pkt);

    void processJoinG(IPv4Address group, IPv4Address rp, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
    void processJoinSG(IPv4Address origin, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
    void processJoinSGrpt(IPv4Address origin, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
    void processPruneG(IPv4Address multGroup, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
    void processPruneSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
    void processPruneSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
    void processAssertSG(PimsmInterface *interface, const AssertMetric& receivedMetric);
    void processAssertG(PimsmInterface *interface, const AssertMetric& receivedMetric);

    // process timers
    void processKeepAliveTimer(cMessage *timer);
    void processRegisterStopTimer(cMessage *timer);
    void processExpiryTimer(cMessage *timer);
    void processJoinTimer(cMessage *timer);
    void processPrunePendingTimer(cMessage *timer);
    void processAssertTimer(cMessage *timer);

    // process signals
    void unroutableMulticastPacketArrived(IPv4Address srcAddr, IPv4Address destAddr);
    void multicastPacketArrivedOnRpfInterface(Route *route);
    void multicastPacketArrivedOnNonRpfInterface(Route *route, int interfaceId);
    void multicastPacketForwarded(IPv4Datagram *datagram);
    void multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group);
    void multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group);

    // internal events
    void joinDesiredChanged(Route *route);
    void designatedRouterAddressHasChanged(InterfaceEntry *ie);
    void iAmDRHasChanged(InterfaceEntry *ie, bool iAmDR);

    // send pim messages
    void sendPIMRegister(IPv4Datagram *datagram, IPv4Address dest, int outInterfaceId);
    void sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource);
    void sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest);
    void sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType JPtype);
    void sendPIMPrune(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType JPtype);
    void sendPIMAssert(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie, bool rptBit);
    void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId, short ttl);
    void forwardMulticastData(IPv4Datagram *datagram, int outInterfaceId);

    // computed intervals
    double joinPruneHoldTime() { return 3.5 * joinPrunePeriod; }    // Holdtime in Join/Prune messages
    double effectivePropagationDelay() { return defaultPropagationDelay; }
    double effectiveOverrideInterval() { return defaultOverrideInterval; }
    double joinPruneOverrideInterval() { return effectivePropagationDelay() + effectiveOverrideInterval(); }

    // update actions
    void updateJoinDesired(Route *route);
    void updateDesignatedRouterAddress(InterfaceEntry *ie);
    void updateCouldAssert(DownstreamInterface *interface);
    void updateAssertTrackingDesired(PimsmInterface *interface);

    // helpers
    bool IamRP(IPv4Address rpAddr) { return rt->isLocalAddress(rpAddr); }
    bool IamDR(InterfaceEntry *ie);
    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
    bool deleteMulticastRoute(Route *route);
    void clearRoutes();
    void cancelAndDeleteTimer(cMessage *& timer);
    void restartTimer(cMessage *timer, double interval);
    void restartExpiryTimer(Route *route, InterfaceEntry *originIntf, int holdTime);

    // routing table access
    bool removeRoute(Route *route);
    Route *findRouteG(IPv4Address group);
    Route *findRouteSG(IPv4Address source, IPv4Address group);
    Route *addNewRouteG(IPv4Address group, int flags);
    Route *addNewRouteSG(IPv4Address source, IPv4Address group, int flags);
    IPv4MulticastRoute *createIPv4Route(Route *route);
    IPv4MulticastRoute *findIPv4Route(IPv4Address source, IPv4Address group);
};

}    // namespace inet

#endif // ifndef __INET_PIMSM_H

