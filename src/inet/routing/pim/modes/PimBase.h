//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMBASE_H
#define __INET_PIMBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/pim/Pim.h"
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/tables/PimInterfaceTable.h"
#include "inet/routing/pim/tables/PimNeighborTable.h"

namespace inet {

/**
 * Base class of PimSm and PimDm modules.
 */
class INET_API PimBase : public RoutingProtocolBase
{
  protected:

    struct AssertMetric {
        short rptBit;
        short preference;
        int metric;
        Ipv4Address address;

        static const AssertMetric PIM_INFINITE;

        AssertMetric() : rptBit(1), preference(-1), metric(0) {}
        AssertMetric(int preference, int metric, Ipv4Address address) :
            rptBit(0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        AssertMetric(bool rptBit, int preference, int metric, Ipv4Address address = Ipv4Address::UNSPECIFIED_ADDRESS)
            : rptBit(rptBit ? 1 : 0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        bool isInfinite() const { return preference == -1; }
        bool operator==(const AssertMetric& other) const;
        bool operator!=(const AssertMetric& other) const;
        bool operator<(const AssertMetric& other) const;
        AssertMetric setAddress(Ipv4Address address) const { return AssertMetric(rptBit, preference, metric, address); }
    };

    struct RouteEntry {
        PimBase *owner;
        Ipv4Address source;
        Ipv4Address group;
        int flags;
        AssertMetric metric; // our metric of the unicast route to the source or RP(group)

        RouteEntry(PimBase *owner, Ipv4Address source, Ipv4Address group)
            : owner(owner), source(source), group(group), flags(0) {}
        virtual ~RouteEntry() {}

        bool isFlagSet(int flag) const { return (flags & flag) != 0; }
        void setFlags(int flags) { this->flags |= flags; }
        void clearFlag(int flag) { flags &= (~flag); }
        void setFlag(int flag, bool value) { if (value) setFlags(flag); else clearFlag(flag); }
    };

    struct Interface {
        RouteEntry *owner;
        NetworkInterface *ie;
        int flags;

        // assert winner state
        enum AssertState { NO_ASSERT_INFO, I_LOST_ASSERT, I_WON_ASSERT };
        AssertState assertState;
        cMessage *assertTimer;
        AssertMetric winnerMetric;

        Interface(RouteEntry *owner, NetworkInterface *ie)
            : owner(owner), ie(ie), flags(0),
            assertState(NO_ASSERT_INFO), assertTimer(nullptr)
        { ASSERT(owner), ASSERT(ie); }
        virtual ~Interface() { owner->owner->cancelAndDelete(assertTimer); }

        bool isFlagSet(int flag) const { return (flags & flag) != 0; }
        void setFlags(int flags) { this->flags |= flags; }
        void clearFlag(int flag) { flags &= (~flag); }
        void setFlag(int flag, bool value) { if (value) setFlags(flag); else clearFlag(flag); }

        void startAssertTimer(double assertTime)
        {
            ASSERT(assertTimer == nullptr);
            assertTimer = new cMessage("PimAssertTimer", AssertTimer);
            assertTimer->setContextPointer(this);
            owner->owner->scheduleAfter(assertTime, assertTimer);
        }

        void deleteAssertInfo()
        {
            assertState = NO_ASSERT_INFO;
            winnerMetric = AssertMetric::PIM_INFINITE;
            owner->owner->cancelAndDelete(assertTimer);
            assertTimer = nullptr;
        }
    };

    struct SourceAndGroup {
        Ipv4Address source;
        Ipv4Address group;

        SourceAndGroup(Ipv4Address source, Ipv4Address group) : source(source), group(group) {}
        bool operator==(const SourceAndGroup& other) const { return source == other.source && group == other.group; }
        bool operator!=(const SourceAndGroup& other) const { return source != other.source || group != other.group; }
        bool operator<(const SourceAndGroup& other) const { return source < other.source || (source == other.source && group < other.group); }
    };

    friend std::ostream& operator<<(std::ostream& out, const SourceAndGroup& sourceGroup);

    enum PimTimerKind {
        // global timers
        HelloTimer = 1,
        TriggeredHelloDelay,

        // timers for each interface and each source-group pair (S,G,I)
        AssertTimer,
        PruneTimer,
        PrunePendingTimer,

        // timers for each source-group pair (S,G)
        GraftRetryTimer,
        UpstreamOverrideTimer,
        PruneLimitTimer,
        SourceActiveTimer,
        StateRefreshTimer,

        // PIM-SM specific timers
        KeepAliveTimer,
        RegisterStopTimer,
        ExpiryTimer,
        JoinTimer,
    };

    static const Ipv4Address ALL_PIM_ROUTERS_MCAST;

  protected:
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<PimInterfaceTable> pimIft;
    ModuleRefByPar<PimNeighborTable> pimNbt;
    opp_component_ptr<Pim> pimModule;

    bool isUp = false;
    bool isEnabled = false;
    const char *hostname = nullptr;

    // parameters
    double helloPeriod = 0;
    double holdTime = 0;
    int designatedRouterPriority = 0;

    PimInterface::PimMode mode = static_cast<PimInterface::PimMode>(0);
    uint32_t generationID = 0;
    cMessage *helloTimer = nullptr;

    // signals
    static simsignal_t sentHelloPkSignal;
    static simsignal_t rcvdHelloPkSignal;

  public:
    PimBase(PimInterface::PimMode mode) : mode(mode) {}
    virtual ~PimBase();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    void sendHelloPackets();
    void sendHelloPacket(PimInterface *pimInterface);
    void processHelloTimer(cMessage *timer);
    void processHelloPacket(Packet *pk);

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

