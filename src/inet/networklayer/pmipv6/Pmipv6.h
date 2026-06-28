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
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class Packet;
class NetworkInterface;
class IInterfaceTable;
class Ipv6RoutingTable;
class Ipv6Route;
class Ipv6NeighbourDiscovery;
class BindingUpdate;
class BindingAcknowledgement;

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
    // role
    bool isLma = false;
    bool isMag = false;

    // module references
    ModuleRefByPar<IInterfaceTable> ift;
    opp_component_ptr<Ipv6RoutingTable> rt6;
    ModuleRefByPar<Ipv6NeighbourDiscovery> ipv6nd;

    // configuration
    Ipv6Address localMobilityAnchorAddress; // MAG: the LMA to register with
    simtime_t bindingLifetime;
    simtime_t advValidLifetime;
    simtime_t advPreferredLifetime;

    //
    // LMA state: the Binding Cache, keyed by Mobile Node Identifier (NAI).
    // Embedded here (rather than a separate module) for simplicity; it can be
    // promoted to a sibling module like the MIPv6 BindingCache if inspection
    // beyond the WATCH below is needed.
    //
    struct BindingCacheEntry {
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 0;
        Ipv6Address servingMagAddress; // the Proxy care-of address (serving MAG)
        unsigned int sequenceNumber = 0;
        simtime_t expiry;
        int tunnelInterfaceId = -1;    // LMA's tunnel to the serving MAG
        Ipv6Route *downlinkRoute = nullptr; // home network prefix -> tunnel
    };
    std::map<std::string, BindingCacheEntry> bindingCache; // key: MN identifier
    std::map<Ipv6Address, int> lmaTunnelByMag;             // serving MAG address -> tunnel interface id (shared by all its MNs)

    //
    // MAG state.
    //
    // A mobile-node policy profile, configured per access interface. The MAG
    // looks one up when a mobile node attaches to an access link.
    struct MobileNodeProfile {
        std::string accessInterfaceName; // empty = match any access interface
        std::string mnIdentifier;
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 64;
    };
    std::vector<MobileNodeProfile> mobileNodeProfiles;

    // an active binding the MAG maintains for a currently-attached mobile node
    struct MagBinding {
        std::string mnIdentifier;
        Ipv6Address homeNetworkPrefix;
        int homeNetworkPrefixLength = 0;
        int accessInterfaceId = -1;
        unsigned int sequenceNumber = 0;
        bool registered = false;
        Ipv6Route *downlinkRoute = nullptr; // home network prefix -> access interface
    };
    std::map<std::string, MagBinding> magBindings; // key: MN identifier
    int magTunnelId = -1;                 // MAG's (shared) tunnel to the LMA
    Ipv6Route *magUplinkRoute = nullptr;  // default route -> tunnel (mobile node uplink)

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
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
    Ipv6Address getEgressAddressFor(const Ipv6Address& destination);
    int getOrCreateTunnel(const Ipv6Address& localEndpoint, const Ipv6Address& remoteEndpoint, std::map<Ipv6Address, int>& tunnelMap);

    // LMA
    void processProxyBindingUpdate(Packet *packet, const BindingUpdate *pbu);

    // MAG
    void parseMobileNodeProfiles();
    void handleMobileNodeAttached(NetworkInterface *accessInterface);
    void handleMobileNodeDetached(NetworkInterface *accessInterface);
    const MobileNodeProfile *findProfileForInterface(NetworkInterface *accessInterface) const;
    void sendProxyBindingUpdate(MagBinding& binding, simtime_t lifetime);
    void processProxyBindingAcknowledgement(Packet *packet, const BindingAcknowledgement *pba);
    void ensureMagTunnel();

  public:
    virtual ~Pmipv6();
};

} // namespace inet

#endif
