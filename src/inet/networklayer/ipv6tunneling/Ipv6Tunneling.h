//
// Copyright (C) 2007
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#ifndef __INET_IPV6TUNNELING_H
#define __INET_IPV6TUNNELING_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"

namespace inet {

/**
 * Disabled placeholder for the former IPv6-in-IPv6 tunnel manager (RFC 2473).
 *
 * The tunnel-management logic has been moved out: the MIPv6 tunnel registry now
 * lives in ~xMIPv6, and the generic tunnel-interface plumbing in ~Ipv6RoutingTable.
 * This module no longer does anything; it is kept temporarily only so the node
 * module structure (and thus the regression fingerprints) stays unchanged while
 * the move is validated, and is removed in a follow-up change.
 */
class INET_API Ipv6Tunneling : public OperationalBase
{
  public:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // lifecycle:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
};

} // namespace inet

#endif
