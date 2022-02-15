//
// Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HOSTAUTOCONFIGURATOR_H
#define __INET_HOSTAUTOCONFIGURATOR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

/**
 * HostAutoConfigurator automatically assigns IP addresses and sets up the
 * routing table for the host it is part of.
 *
 * For more info please see the NED file.
 *
 * @author Christoph Sommer
 */
class INET_API HostAutoConfigurator : public OperationalBase
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

  public:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void handleMessageWhenUp(cMessage *msg) override;

  protected:
    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_CONFIGURATION; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }

    virtual void setupNetworkLayer();
};

} // namespace inet

#endif

