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
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
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
        L3Address address;

        static const AssertMetric PIM_INFINITE;

        AssertMetric() : rptBit(1), preference(-1), metric(0) {}
        AssertMetric(int preference, int metric, L3Address address) :
            rptBit(0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        AssertMetric(bool rptBit, int preference, int metric, L3Address address = L3Address())
            : rptBit(rptBit ? 1 : 0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
        bool isInfinite() const { return preference == -1; }
        bool operator==(const AssertMetric& other) const;
        bool operator!=(const AssertMetric& other) const;
        bool operator<(const AssertMetric& other) const;
        AssertMetric setAddress(L3Address address) const { return AssertMetric(rptBit, preference, metric, address); }
    };

    struct RouteEntry {
        PimBase *owner;
        L3Address source;
        L3Address group;
        int flags;
        AssertMetric metric; // our metric of the unicast route to the source or RP(group)

        RouteEntry(PimBase *owner, L3Address source, L3Address group)
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
        L3Address source;
        L3Address group;

        SourceAndGroup(L3Address source, L3Address group) : source(source), group(group) {}
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

  protected:
    ModuleRefByPar<IRoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<PimInterfaceTable> pimIft;
    ModuleRefByPar<PimNeighborTable> pimNbt;
    opp_component_ptr<Pim> pimModule;

    // address family: ipv4 or ipv6
    const Protocol *networkProtocol = &Protocol::ipv4;
    L3Address allPimRoutersMcast; // 224.0.0.13 (IPv4) or ff02::d (IPv6)

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

    // RFC 4601 4.9.1 Encoded-Address field lengths -- they depend on the address
    // family (IPv4: 4-byte addresses, IPv6: 16-byte addresses). The IPv4 sizes are
    // the ENCODED_*_ADDRESS_LENGTH constants from PimPacket_m.h.
    B encodedUnicastAddressLength() const { return isIpv6() ? B(2 + 16) : ENCODED_UNICODE_ADDRESS_LENGTH; }
    B encodedGroupAddressLength() const { return isIpv6() ? B(4 + 16) : ENCODED_GROUP_ADDRESS_LENGTH; }
    B encodedSourceAddressLength() const { return isIpv6() ? B(4 + 16) : ENCODED_SOURCE_ADDRESS_LENGTH; }

    // address-family helpers: dispatch on networkProtocol to Ipv4InterfaceData / Ipv6InterfaceData
    bool isIpv6() const { return networkProtocol == &Protocol::ipv6; }
    L3Address getInterfaceAddress(NetworkInterface *ie) const;
    void joinMulticastGroup(NetworkInterface *ie, const L3Address& group);
    bool hasMulticastListener(NetworkInterface *ie, const L3Address& group) const;
    bool isMemberOfMulticastGroup(NetworkInterface *ie, const L3Address& group) const;

    // address-family helpers for multicast routes and signal payloads:
    // dispatch on networkProtocol to the Ipv4/Ipv6 multicast-route and header types
    IMulticastRoute *createMulticastRoute();
    IMulticastRoute *findMulticastRoute(L3Address group, L3Address source);
    static NetworkInterface *getInInterface(IMulticastRoute *route);
    static bool hasOutInterface(IMulticastRoute *route, const NetworkInterface *ie);
    static unsigned int getAdminDist(IRoute *route);
    bool isRoutableMulticastSource(const L3Address& srcAddr) const;
    bool isRoutableMulticastGroup(const L3Address& group) const;
    bool isSsmGroup(const L3Address& group) const;
    void getMulticastPacketAddresses(cObject *obj, L3Address& srcAddr, L3Address& destAddr, unsigned short& ttl) const;
    void getMulticastGroupInfo(cObject *obj, NetworkInterface *& ie, L3Address& groupAddress) const;
    void getMulticastListenerSources(cObject *obj, NetworkInterface *& ie, L3Address& groupAddress, McastSourceFilterMode& filterMode, std::vector<L3Address>& sources) const;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

