//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

#ifndef __INET_SIMPLECCBATTERY_H
#define __INET_SIMPLECCBATTERY_H

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/power/base/CcEnergyStorageBase.h"

namespace inet {

namespace power {

class INET_API SimpleCcBattery : public CcEnergyStorageBase
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
    LifecycleController lifecycleController;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;

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

    virtual C getNominalChargeCapacity() const override{ return nominalCapacity; }
    virtual C getResidualChargeCapacity() const override { return residualCapacity; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_SIMPLECCBATTERY_H

