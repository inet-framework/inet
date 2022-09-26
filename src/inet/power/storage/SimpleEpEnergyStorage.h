//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLEEPENERGYSTORAGE_H
#define __INET_SIMPLEEPENERGYSTORAGE_H

#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/power/base/EpEnergyStorageBase.h"

namespace inet {

namespace power {

/**
 * This class implements a simple total power integrating energy storage.
 *
 * It maintains its residual capacity by repeatedly scheduling a single timer to
 * the next capacity update. The next update happens when either the storage
 * becomes completely depleted, completely charged, or the next capacity report
 * is done. Besides, it immediately updates the capacity when the total absorbed
 * or provided power changes, and it also reschedules the timer.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API SimpleEpEnergyStorage : public EpEnergyStorageBase, public StringFormat::IDirectiveResolver, public LifecycleController
{
  protected:
    /**
     * The nominal capacity is in the range [0, +infinity).
     */
    J nominalCapacity = J(NaN);

    /**
     * The residual capacity is in the range [0, nominalCapacity].
     */
    J residualCapacity = J(NaN);

    /**
     * Specifies the amount of capacity change which will be reported.
     */
    J printCapacityStep = J(NaN);

    /**
     * The simulation time when the residual capacity was last updated.
     */
    simtime_t lastResidualCapacityUpdate = -1;

    /**
     * The timer that is scheduled to the earliest time when the energy storage
     * will be depleted, the energy storage will be charged.
     */
    cMessage *timer = nullptr;

    /**
     * The capacity that will be set when the timer expires.
     */
    J targetCapacity = J(NaN);

    /**
     * The containing node module.
     */
    cModule *networkNode = nullptr;

    /**
     * The status of the node.
     */
    NodeStatus *nodeStatus = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual std::string resolveDirective(char directive) const override;

    virtual void updateTotalPowerConsumption() override;
    virtual void updateTotalPowerGeneration() override;

    virtual void executeNodeOperation(J newResidualCapacity);
    virtual void setResidualCapacity(J newResidualCapacity);
    virtual void updateResidualCapacity();
    virtual void scheduleTimer();

  public:
    virtual ~SimpleEpEnergyStorage();

    virtual J getNominalEnergyCapacity() const override { return nominalCapacity; }
    virtual J getResidualEnergyCapacity() const override;
};

} // namespace power

} // namespace inet

#endif

