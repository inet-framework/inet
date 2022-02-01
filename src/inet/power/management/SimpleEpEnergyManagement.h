//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLEEPENERGYMANAGEMENT_H
#define __INET_SIMPLEEPENERGYMANAGEMENT_H

#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/power/contract/IEpEnergyManagement.h"
#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {

namespace power {

class INET_API SimpleEpEnergyManagement : public cSimpleModule, public virtual IEpEnergyManagement, public cListener, public LifecycleController
{
  protected:
    // parameters
    J nodeShutdownCapacity = J(NaN);
    J nodeStartCapacity = J(NaN);
    IEpEnergyStorage *energyStorage = nullptr;

    // state
    cModule *networkNode = nullptr;
    NodeStatus *nodeStatus = nullptr;
    cMessage *lifecycleOperationTimer = nullptr;
    J targetCapacity = J(NaN);

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;

    virtual void executeNodeOperation(J estimatedEnergyCapacity);
    virtual void scheduleLifecycleOperationTimer();

  public:
    virtual ~SimpleEpEnergyManagement();

    virtual IEnergyStorage *getEnergyStorage() const override { return energyStorage; }
    virtual J getEstimatedEnergyCapacity() const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace power

} // namespace inet

#endif

