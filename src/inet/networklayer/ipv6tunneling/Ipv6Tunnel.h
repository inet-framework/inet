//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV6TUNNEL_H
#define __INET_IPV6TUNNEL_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

/**
 * A virtual network interface performing IPv6-in-IPv6 (RFC 2473) encapsulation.
 *
 * A packet routed to this interface is handed to the IPv6 layer's *service*
 * (upper) interface with an L3AddressReq for the tunnel endpoints, so IPv6
 * encapsulates it (outer source -> destination, next header = IPv6) and routes
 * the resulting datagram toward the tunnel exit as a locally-originated packet.
 *
 * The IPv6 core needs no tunneling-specific knowledge: this is just the normal
 * "encapsulate an upper-layer payload" path, where the payload happens to be an
 * IPv6 datagram. This mirrors the IPv4-style virtual tunnel interfaces.
 */
class INET_API Ipv6Tunnel : public LayeredProtocolBase
{
  protected:
    opp_component_ptr<NetworkInterface> networkInterface;

    Ipv6Address source; // tunnel entry point (an address of this node)
    Ipv6Address destination; // tunnel exit point

    int upperLayerInGateId = -1;
    int upperLayerOutGateId = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void configureNetworkInterface();

    virtual void handleUpperPacket(Packet *packet) override;

    virtual bool isUpperMessage(cMessage *message) const override { return message->getArrivalGateId() == upperLayerInGateId; }
    virtual bool isLowerMessage(cMessage *message) const override { return false; }

    // lifecycle
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
};

} // namespace inet

#endif
