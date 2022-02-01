//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Tomas Prochazka (xproch21@stud.fit.vutbr.cz),
//          Vladimir Vesely (ivesely@fit.vutbr.cz), Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMSM_H
#define __INET_PIMSM_H

#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/routing/pim/modes/PimBase.h"

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
 * of the Ipv4RoutingTable. The RFC defines the terms "Tree Information Base" (TIB)
 * and "Multicast Forwaring Information Base" (MFIB). According to this division,
 * TIB is the state of this module, while MFIB is stored by Ipv4RoutingTable.
 *
 * Incoming packets, notifications, and timer events may cause a change
 * of the TIB, and of the MFIB (if the forwarding rules change).
 *
 *
 */
class INET_API PimSm : public PimBase, protected cListener
{
  private:
    struct Route;

    struct PimsmInterface : public Interface {
        cMessage *expiryTimer;

        enum Flags {
            RECEIVER_INCLUDE        = 1 << 0, // local_receiver_include(S,G,I) or local_receiver_include(*,G,I)
            RECEIVER_EXCLUDE        = 1 << 1, // local_receiver_exclude(S,G,I)
            COULD_ASSERT            = 1 << 2, // CouldAssert(S,G,I)
            ASSERT_TRACKING_DESIRED = 1 << 3, // AssertTrackingDesired(S,G,I)
        };

        PimsmInterface(Route *owner, NetworkInterface *ie);
        virtual ~PimsmInterface();
        Route *route() const { return check_and_cast<Route *>(owner); }
        PimSm *pimsm() const { return check_and_cast<PimSm *>(owner->owner); }
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
                   (( /*I_am_DR AND*/ assertState != I_LOST_ASSERT) || assertState == I_WON_ASSERT);
        }

        bool pimExclude() const
        {
            return localReceiverExclude() &&
                   (( /*I_am_DR AND*/ assertState != I_LOST_ASSERT) || assertState == I_WON_ASSERT);
        }
    };

    // upstream interface is toward the RP or toward the source
    struct UpstreamInterface : public PimsmInterface {
        Ipv4Address nextHop; // RPF nexthop, <unspec> at the DR in (S,G) routes

        UpstreamInterface(Route *owner, NetworkInterface *ie, Ipv4Address nextHop)
            : PimsmInterface(owner, ie), nextHop(nextHop) {}
        int getInterfaceId() const { return ie->getInterfaceId(); }
        Ipv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
    };

    struct DownstreamInterface : public PimsmInterface {
        /** States of each outgoing interface. */
        enum JoinPruneState { NO_INFO, JOIN, PRUNE_PENDING };

        JoinPruneState joinPruneState;
        cMessage *prunePendingTimer;

        DownstreamInterface(Route *owner, NetworkInterface *ie, JoinPruneState joinPruneState, bool show = true)
            : PimsmInterface(owner, ie), joinPruneState(joinPruneState), prunePendingTimer(nullptr) {}
        virtual ~DownstreamInterface();

        int getInterfaceId() const { return ie->getInterfaceId(); }
        bool isInImmediateOlist() const;
        bool isInInheritedOlist() const;
        void startPrunePendingTimer(double joinPruneOverrideInterval);
    };

    typedef std::vector<DownstreamInterface *> DownstreamInterfaceVector;

    class PimSmOutInterface : public IMulticastRoute::OutInterface {
        DownstreamInterface *downstream;

      public:
        PimSmOutInterface(DownstreamInterface *downstream)
            : OutInterface(downstream->ie), downstream(downstream) {}
        virtual bool isEnabled() override { return downstream->isInInheritedOlist(); }
    };

    enum RouteType {
        RP, // (*,*,RP)
        G, // (*,G)
        SG, // (S,G)
        SGrpt // (S,G,rpt)
    };

    // Holds (*,G), (S,G) or (S,G,rpt) state
    struct Route : public RouteEntry {
        enum Flags {
            PRUNED                    = 0x01, // UpstreamJPState
            REGISTER                  = 0x02, // Register flag
            SPT_BIT                   = 0x04, // used to distinguish whether to forward on (*,*,RP)/(*,G) or on (S,G) state
            JOIN_DESIRED              = 0x08,
            SOURCE_DIRECTLY_CONNECTED = 0x10
        };

        RouteType type;
        Ipv4Address rpAddr;

        // related routes
        Route *rpRoute;
        Route *gRoute;
        Route *sgrptRoute;

        // Originated from destination.Ensures loop freeness.
        unsigned int sequencenumber;
        // Time of routing table entry creation
        simtime_t installtime; // TODO not used

        cMessage *keepAliveTimer; // only for (S,G) routes
        cMessage *joinTimer;

        // Register state (only for (S,G) at the DR)
        enum RegisterState { RS_NO_INFO, RS_JOIN, RS_PRUNE, RS_JOIN_PENDING };
        RegisterState registerState;
        cMessage *registerStopTimer;

        // interface specific state
        UpstreamInterface *upstreamInterface; // may be nullptr at RP and at DR
        DownstreamInterfaceVector downstreamInterfaces; ///< Out interfaces (downstream)

      public:
        Route(PimSm *owner, RouteType type, Ipv4Address origin, Ipv4Address group);
        virtual ~Route();
        PimSm *pimsm() const { return check_and_cast<PimSm *>(owner); }

        void addDownstreamInterface(DownstreamInterface *outInterface);
        void removeDownstreamInterface(unsigned int i);

        DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId);
        DownstreamInterface *getDownstreamInterfaceByInterfaceId(int interfaceId);
        int findDownstreamInterface(NetworkInterface *ie);

        bool isImmediateOlistNull();
        bool isInheritedOlistNull();
        bool joinDesired() const { return isFlagSet(JOIN_DESIRED); }
        bool isSourceDirectlyConnected() const { return isFlagSet(SOURCE_DIRECTLY_CONNECTED); }

        void startKeepAliveTimer(double keepAlivePeriod);
        void startRegisterStopTimer(double interval);
        void startJoinTimer(double joinPrunePeriod);
    };

    friend std::ostream& operator<<(std::ostream& out, const PimSm::Route& sourceGroup);

    typedef std::map<SourceAndGroup, Route *> RoutingTable;

    // parameters
    Ipv4Address rpAddr;
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
    PimSm() : PimBase(PimInterface::SparseMode) {}
    virtual ~PimSm();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void stopPIMRouting();
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  private:
    // process PIM messages
    void processJoinPrunePacket(Packet *pk);
    void processRegisterPacket(Packet *pk);
    void processRegisterStopPacket(Packet *pk);
    void processAssertPacket(Packet *pk);

    void processJoinG(Ipv4Address group, Ipv4Address rp, Ipv4Address upstreamNeighborField, int holdTime, NetworkInterface *inInterface);
    void processJoinSG(Ipv4Address origin, Ipv4Address group, Ipv4Address upstreamNeighborField, int holdTime, NetworkInterface *inInterface);
    void processJoinSGrpt(Ipv4Address origin, Ipv4Address group, Ipv4Address upstreamNeighborField, int holdTime, NetworkInterface *inInterface);
    void processPruneG(Ipv4Address multGroup, Ipv4Address upstreamNeighborField, NetworkInterface *inInterface);
    void processPruneSG(Ipv4Address source, Ipv4Address group, Ipv4Address upstreamNeighborField, NetworkInterface *inInterface);
    void processPruneSGrpt(Ipv4Address source, Ipv4Address group, Ipv4Address upstreamNeighborField, NetworkInterface *inInterface);
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
    void unroutableMulticastPacketArrived(Ipv4Address srcAddr, Ipv4Address destAddr);
    void multicastPacketArrivedOnRpfInterface(Route *route);
    void multicastPacketArrivedOnNonRpfInterface(Route *route, int interfaceId);
    void multicastPacketForwarded(Packet *pk); // pk should begin with Ipv4Header
    void multicastReceiverAdded(NetworkInterface *ie, Ipv4Address group);
    void multicastReceiverRemoved(NetworkInterface *ie, Ipv4Address group);

    // internal events
    void joinDesiredChanged(Route *route);
    void designatedRouterAddressHasChanged(NetworkInterface *ie);
    void iAmDRHasChanged(NetworkInterface *ie, bool iAmDR);

    // send pim messages
    void sendPIMRegister(Packet *pk, Ipv4Address dest, int outInterfaceId); // pk should begin with Ipv4Header
    void sendPIMRegisterStop(Ipv4Address source, Ipv4Address dest, Ipv4Address multGroup, Ipv4Address multSource);
    void sendPIMRegisterNull(Ipv4Address multSource, Ipv4Address multDest);
    void sendPIMJoin(Ipv4Address group, Ipv4Address source, Ipv4Address upstreamNeighbor, RouteType JPtype);
    void sendPIMPrune(Ipv4Address group, Ipv4Address source, Ipv4Address upstreamNeighbor, RouteType JPtype);
    void sendPIMAssert(Ipv4Address source, Ipv4Address group, AssertMetric metric, NetworkInterface *ie, bool rptBit);
    void sendToIP(Packet *packet, Ipv4Address source, Ipv4Address dest, int outInterfaceId, short ttl);
    void forwardMulticastData(Packet *pk, int outInterfaceId); // pk should begin with Ipv4Header

    // computed intervals
    double joinPruneHoldTime() { return 3.5 * joinPrunePeriod; } // Holdtime in Join/Prune messages
    double effectivePropagationDelay() { return defaultPropagationDelay; }
    double effectiveOverrideInterval() { return defaultOverrideInterval; }
    double joinPruneOverrideInterval() { return effectivePropagationDelay() + effectiveOverrideInterval(); }

    // update actions
    void updateJoinDesired(Route *route);
    void updateDesignatedRouterAddress(NetworkInterface *ie);
    void updateCouldAssert(DownstreamInterface *interface);
    void updateAssertTrackingDesired(PimsmInterface *interface);

    // helpers
    bool IamRP(Ipv4Address rpAddr) { return rt->isLocalAddress(rpAddr); }
    bool IamDR(NetworkInterface *ie);
    PimInterface *getIncomingInterface(NetworkInterface *fromIE);
    bool deleteMulticastRoute(Route *route);
    void clearRoutes();
    void cancelAndDeleteTimer(cMessage *& timer);
    void restartTimer(cMessage *timer, double interval);
    void restartExpiryTimer(Route *route, NetworkInterface *originIntf, int holdTime);

    // routing table access
    bool removeRoute(Route *route);
    Route *findRouteG(Ipv4Address group);
    Route *findRouteSG(Ipv4Address source, Ipv4Address group);
    Route *addNewRouteG(Ipv4Address group, int flags);
    Route *addNewRouteSG(Ipv4Address source, Ipv4Address group, int flags);
    Ipv4MulticastRoute *createIpv4Route(Route *route);
    Ipv4MulticastRoute *findIpv4Route(Ipv4Address source, Ipv4Address group);
};

} // namespace inet

#endif

