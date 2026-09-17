//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PMIPV6_H
#define __INET_PMIPV6_H

#include <map>
#include <string>
#include <vector>

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class Packet;
class NetworkInterface;
class IInterfaceTable;
class Ipv6RoutingTable;
class Ipv6Route;
class Ipv6NeighbourDiscovery;

/**
 * Implements Proxy Mobile IPv6 (RFC 5213): network-based mobility management in
 * which the network, rather than the mobile node, manages mobility. The same
 * module implements both PMIPv6 roles, selected by parameters:
 *
 *  - Local Mobility Anchor (LMA): the topological anchor for a mobile node's
 *    home network prefix. It maintains the Binding Cache, owns one end of the
 *    bidirectional tunnel to each serving Mobile Access Gateway, and routes a
 *    mobile node's home network prefix into the tunnel toward its current MAG.
 *
 *  - Mobile Access Gateway (MAG): the access router a mobile node attaches to.
 *    It detects attachment/detachment at the link layer, sends Proxy Binding
 *    Updates to the LMA on behalf of the (unmodified) mobile node, sets up the
 *    tunnel to the LMA, and advertises the mobile node's home network prefix on
 *    the access link so the mobile node keeps its address as it roams.
 *
 * The mobile node itself runs no mobility software: it is a plain IPv6 host that
 * performs stateless address autoconfiguration from the prefix the MAG advertises.
 */
class INET_API Pmipv6 : public OperationalBase, protected cListener
{
  protected:
    // signals
    static simsignal_t proxyBindingUpdateSentSignal;
    static simsignal_t proxyBindingAcknowledgementReceivedSignal;
    static simsignal_t proxyBindingUpdateReceivedSignal;
    static simsignal_t homeNetworkPrefixReanchoredSignal;
    static simsignal_t mobileNodeDetachedSignal;
    static simsignal_t bindingCacheSizeSignal;

    // role
    bool isLma = false;
    bool isMag = false;

    // module references
    ModuleRefByPar<IInterfaceTable> ift;
    opp_component_ptr<Ipv6RoutingTable> rt6;
    ModuleRefByPar<Ipv6NeighbourDiscovery> ipv6nd;

    // configuration
    Ipv6Address localMobilityAnchorAddress; // MAG: the LMA to register with
    bool timestampBasedOrdering = true;
    bool detectTransmissionFailure = false;
    simtime_t detachDetectionTimeout;
    simtime_t presenceCheckInterval;
    simtime_t initialBindingAckTimeout;
    simtime_t maxBindingAckTimeout;
    simtime_t minDelayBeforeBindingCacheEntryDelete;
    simtime_t bindingLifetime;
    simtime_t maxBindingLifetime;
    double bindingRefreshRatio = 0;
    simtime_t advValidLifetime;
    simtime_t advPreferredLifetime;

    //
    // LMA state: the Binding Cache, keyed by Mobile Node Identifier (NAI).
    // Embedded here (rather than a separate module) for simplicity; it can be
    // promoted to a sibling module like the MIPv6 BindingCache if it ever needs
    // more than the watches registered in initialize(). This struct and the two
    // MAG ones below are public so that the ostream operators those watches use
    // can name them.
    //
  public:
    //
    // A mobility session is named by the tuple RFC 5213 Section 5.4.1.2 looks a
    // Binding Cache entry up by: the mobile node's identifier, the access
    // technology it attached through, and the link-layer identifier of the
    // attached interface. Two mobile nodes on one access link differ in the
    // third component even when the first two coincide. That lookup applies when a
    // request names no home network prefix; when it names one, Section 5.4.1.1
    // finds the entry by the prefix instead, and that is the usual case here.
    //
    struct MobilitySessionKey {
        std::string mnIdentifier;
        uint8_t accessTechnologyType = 0;
        MacAddress mnLinkLayerIdentifier; // unspecified = not known
        bool operator<(const MobilitySessionKey& other) const;
    };

    // Self-messages carry the mobility session they belong to; the kind says what to do.
    class INET_API Pmipv6Timer : public cMessage {
      public:
        MobilitySessionKey session;
        Pmipv6Timer(const char *name, short kind) : cMessage(name, kind) {}
    };

    enum TimerKind {
        MAG_PRESENCE_CHECK = 1, // one per gateway: evaluates detachDetectionTimeout
        LMA_BINDING_DELETE,     // one per deregistered binding: the deletion delay
        MAG_BINDING_RETRANSMIT, // one per binding: an unanswered Proxy Binding Update
        MAG_BINDING_REFRESH,    // one per binding: time to re-register before the lifetime runs out
        LMA_BINDING_EXPIRY,     // one per binding: the granted lifetime ran out
    };

    struct BindingCacheEntry {
        MobilitySessionKey session;
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 0;
        Ipv6Address servingMagAddress; // the Proxy care-of address (serving MAG)
        unsigned int sequenceNumber = 0;
        uint64_t timestamp = 0;        // of the most recently accepted Proxy Binding Update
        simtime_t expiry;
        int tunnelInterfaceId = -1;    // LMA's tunnel to the serving MAG
        Ipv6Route *downlinkRoute = nullptr; // home network prefix -> tunnel
        Pmipv6Timer *deleteTimer = nullptr; // running while a deregistration is being held
        Pmipv6Timer *expiryTimer = nullptr; // running for the granted lifetime
    };

  protected:
    typedef std::map<MobilitySessionKey, BindingCacheEntry> BindingCache;
    BindingCache bindingCache;
    std::map<Ipv6Address, int> lmaTunnelByMag;             // serving MAG address -> tunnel interface id (shared by all its MNs)

    //
    // MAG state.
    //
    // A mobile-node policy profile, configured per access interface. The MAG
    // looks one up when a mobile node attaches to an access link.
  public:
    struct MobileNodeProfile {
        MacAddress linkLayerAddress;     // unspecified = match any station
        std::string accessInterfaceName; // empty = match any access interface
        std::string mnIdentifier;
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 64;
    };

    // an active binding the MAG maintains for a currently-attached mobile node
    struct MagBinding {
        std::string mnIdentifier;
        MacAddress mnLinkLayerIdentifier; // the attached interface of the mobile node
        uint8_t accessTechnologyType = 0;
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 0;
        int accessInterfaceId = -1;       // RFC 5213 Section 6.1 calls this the if-id
        unsigned int sequenceNumber = 0;
        bool registered = false;
        bool deregistering = false;   // a lifetime-0 Proxy Binding Update is outstanding
        bool detached = false;        // the access link reported the node gone
        simtime_t lastPresence;       // when the gateway last had evidence of the node
        Ipv6Route *downlinkRoute = nullptr; // home network prefix -> access interface
        // the Proxy Binding Update awaiting an acknowledgement, and its back-off
        Pmipv6Timer *retransmitTimer = nullptr;
        Pmipv6Timer *refreshTimer = nullptr;
        simtime_t retransmitInterval;
        simtime_t pendingLifetime;
        uint8_t pendingHandoffIndicator = 0;
    };

  protected:
    std::vector<MobileNodeProfile> mobileNodeProfiles;
    std::map<std::string, MagBinding> magBindings; // key: MN identifier
    cMessage *presenceCheckTimer = nullptr;
    int magTunnelId = -1;                 // MAG's (shared) tunnel to the LMA
    Ipv6Route *magUplinkRoute = nullptr;  // default route -> tunnel (mobile node uplink)

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleTimer(cMessage *timer);
    using cListener::receiveSignal;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // lifecycle
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // common
    void processMobilityMessage(Packet *packet);
    void sendMobilityMessage(Packet *packet, const Ipv6Address& destAddress, const Ipv6Address& srcAddress);
    void dropPacket(Packet *packet, PacketDropReason reason);
    Ipv6Address getEgressAddressFor(const Ipv6Address& destination);
    int getOrCreateTunnel(const Ipv6Address& localEndpoint, const Ipv6Address& remoteEndpoint, std::map<Ipv6Address, int>& tunnelMap);

    // LMA
    void processProxyBindingUpdate(Packet *packet, const BindingUpdate *pbu);
    BindingCache::iterator lookupBindingCacheEntry(const BindingUpdate *pbu);
    void sendProxyBindingAcknowledgement(const BindingUpdate *pbu, BaStatus status,
            unsigned int lifetime, uint64_t timestamp, const Ipv6Address& magAddress, const Ipv6Address& lmaAddress);
    void deleteBindingCacheEntry(BindingCache::iterator it);

    // MAG
    void parseMobileNodeProfiles();
    void handleMobileNodeAttached(NetworkInterface *accessInterface, const MacAddress& stationAddress);
    void handleMobileNodeDetached(NetworkInterface *accessInterface, const MacAddress& stationAddress);
    const MobileNodeProfile *findProfile(NetworkInterface *accessInterface, const MacAddress& stationAddress) const;
    MagBinding *findBinding(int accessInterfaceId, const MacAddress& stationAddress);
    bool isMobileNodePresent(const MagBinding& binding) const;
    void noteMobileNodePresence(int accessInterfaceId, const MacAddress& stationAddress);
    void checkMobileNodePresence();
    void deregisterMobileNode(MagBinding& binding);
    void releaseMagBinding(MagBinding& binding);
    void withdrawHomeNetworkPrefix(MagBinding& binding);
    void retransmitProxyBindingUpdate(const MobilitySessionKey& session);
    void refreshProxyBinding(const MobilitySessionKey& session);
    void sendProxyBindingUpdate(MagBinding& binding, simtime_t lifetime, uint8_t handoffIndicator);
    void processProxyBindingAcknowledgement(Packet *packet, const BindingAcknowledgement *pba);
    void ensureMagTunnel();

  public:
    virtual ~Pmipv6();
};

std::ostream& operator<<(std::ostream& os, const Pmipv6::MobilitySessionKey& key);
std::ostream& operator<<(std::ostream& os, const Pmipv6::BindingCacheEntry& entry);
std::ostream& operator<<(std::ostream& os, const Pmipv6::MobileNodeProfile& profile);
std::ostream& operator<<(std::ostream& os, const Pmipv6::MagBinding& binding);

} // namespace inet

#endif
