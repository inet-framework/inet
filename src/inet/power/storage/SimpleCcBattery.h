//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLECCBATTERY_H
#define __INET_SIMPLECCBATTERY_H

#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/power/base/CcEnergyStorageBase.h"

namespace inet {

namespace power {

class INET_API SimpleCcBattery : public CcEnergyStorageBase, public StringFormat::IDirectiveResolver, public LifecycleController
{
  protected:
    // parameters
    C nominalCapacity = C(NaN);
    V nominalVoltage = V(NaN);
    Ohm internalResistance = Ohm(NaN);

    // state
    C residualCapacity = C(NaN);
    simtime_t lastResidualCapacityUpdate = -1;

    cModule *networkNode = nullptr;
    NodeStatus *nodeStatus = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual std::string resolveDirective(char directive) const override;

    virtual void updateTotalCurrentConsumption() override;
    virtual void updateTotalCurrentGeneration() override;

    virtual void setResidualCapacity(C newResidualCapacity);
    virtual void updateResidualCapacity();

    virtual void executeNodeOperation(C newResidualCapacity);

  public:
    virtual V getNominalVoltage() const override { return nominalVoltage; }
    virtual V getOutputVoltage() const override { return nominalVoltage + (totalCurrentGeneration - totalCurrentConsumption) * internalResistance; }

    virtual A getTotalCurrentConsumption() const override { return totalCurrentConsumption; }
    virtual A getTotalCurrentGeneration() const override { return totalCurrentGeneration; }

    virtual C getNominalChargeCapacity() const override { return nominalCapacity; }
    virtual C getResidualChargeCapacity() const override { return residualCapacity; }
};

} // namespace power

} // namespace inet

#endif

