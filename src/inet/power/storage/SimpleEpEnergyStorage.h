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

#ifndef __INET_SIMPLEEPENERGYSTORAGE_H
#define __INET_SIMPLEEPENERGYSTORAGE_H

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
 * @author Levente Meszaros
 */
class INET_API SimpleEpEnergyStorage : public EpEnergyStorageBase
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

    /**
     * LifecycleController.
     */
    LifecycleController lifecycleController;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;

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

#endif // ifndef __INET_SIMPLEEPENERGYSTORAGE_H

