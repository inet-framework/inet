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

#ifndef __INET_PIMBASE_H
#define __INET_PIMBASE_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/routing/pim/tables/PIMNeighborTable.h"
#include "inet/routing/pim/tables/PIMInterfaceTable.h"
#include "inet/routing/pim/PIMPacket.h"

namespace inet {

/**
 * Base class of PIMSM and PIMDM modules.
 */
class INET_API PIMBase : public OperationalBase
{
  protected:

    struct AssertMetric
    {
        short rptBit;
        short preference;
        int metric;
        IPv4Address address;

        static const AssertMetric PIM_INFINITE;

        AssertMetric() : rptBit(1), preference(-1), metric(0) {}
        AssertMetric(int preference, int metric, IPv4Address address) :
            rptBit(0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        AssertMetric(bool rptBit, int preference, int metric, IPv4Address address = IPv4Address::UNSPECIFIED_ADDRESS)
            : rptBit(rptBit ? 1 : 0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        bool isInfinite() const { return preference == -1; }
        bool operator==(const AssertMetric& other) const;
        bool operator!=(const AssertMetric& other) const;
        bool operator<(const AssertMetric& other) const;
        AssertMetric setAddress(IPv4Address address) const { return AssertMetric(rptBit, preference, metric, address); }
    };

    struct RouteEntry
    {
        PIMBase *owner;
        IPv4Address source;
        IPv4Address group;
        int flags;
        AssertMetric metric;    // our metric of the unicast route to the source or RP(group)

        RouteEntry(PIMBase *owner, IPv4Address source, IPv4Address group)
            : owner(owner), source(source), group(group), flags(0) {}
        virtual ~RouteEntry() {};

        bool isFlagSet(int flag) const { return (flags & flag) != 0; }
        void setFlags(int flags) { this->flags |= flags; }
        void clearFlag(int flag) { flags &= (~flag); }
        void setFlag(int flag, bool value) { if (value) setFlags(flag); else clearFlag(flag); }
    };

    struct Interface
    {
        RouteEntry *owner;
        InterfaceEntry *ie;
        int flags;

        // assert winner state
        enum AssertState { NO_ASSERT_INFO, I_LOST_ASSERT, I_WON_ASSERT };
        AssertState assertState;
        cMessage *assertTimer;
        AssertMetric winnerMetric;

        Interface(RouteEntry *owner, InterfaceEntry *ie)
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
            owner->owner->scheduleAt(simTime() + assertTime, assertTimer);
        }

        void deleteAssertInfo()
        {
            assertState = NO_ASSERT_INFO;
            winnerMetric = AssertMetric::PIM_INFINITE;
            owner->owner->cancelAndDelete(assertTimer);
            assertTimer = nullptr;
        }
    };

    struct SourceAndGroup
    {
        IPv4Address source;
        IPv4Address group;

        SourceAndGroup(IPv4Address source, IPv4Address group) : source(source), group(group) {}
        bool operator==(const SourceAndGroup& other) const { return source == other.source && group == other.group; }
        bool operator!=(const SourceAndGroup& other) const { return source != other.source || group != other.group; }
        bool operator<(const SourceAndGroup& other) const { return source < other.source || (source == other.source && group < other.group); }
    };

    enum PIMTimerKind {
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

        //PIM-SM specific timers
        KeepAliveTimer,
        RegisterStopTimer,
        ExpiryTimer,
        JoinTimer,
    };

    static const IPv4Address ALL_PIM_ROUTERS_MCAST;

  protected:
    IIPv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    PIMInterfaceTable *pimIft = nullptr;
    PIMNeighborTable *pimNbt = nullptr;

    bool isUp = false;
    bool isEnabled = false;
    const char *hostname = nullptr;

    // parameters
    double helloPeriod = 0;
    double holdTime = 0;
    int designatedRouterPriority = 0;

    PIMInterface::PIMMode mode = (PIMInterface::PIMMode)0;
    uint32_t generationID = 0;
    cMessage *helloTimer = nullptr;

    // signals
    static simsignal_t sentHelloPkSignal;
    static simsignal_t rcvdHelloPkSignal;

  public:
    PIMBase(PIMInterface::PIMMode mode) : mode(mode) {}
    virtual ~PIMBase();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    void sendHelloPackets();
    void sendHelloPacket(PIMInterface *pimInterface);
    void processHelloTimer(cMessage *timer);
    void processHelloPacket(PIMHello *pkt);

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_ROUTING_PROTOCOLS; }
    virtual bool isNodeStartStage(int stage) override { return stage == INITSTAGE_ROUTING_PROTOCOLS; }
    virtual bool isNodeShutdownStage(int stage) override { return stage == INITSTAGE_ROUTING_PROTOCOLS; }
    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;
};

}    // namespace inet

#endif // ifndef __INET_PIMBASE_H

