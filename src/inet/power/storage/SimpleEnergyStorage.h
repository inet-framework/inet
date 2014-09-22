//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_SIMPLEENERGYSTORAGE_H
#define __INET_SIMPLEENERGYSTORAGE_H

#include "inet/power/base/EnergyStorageBase.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/NodeStatus.h"

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
 * Node shutdown and node start is done by executing the appropriate operation
 * using the lifecycle controller.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API SimpleEnergyStorage : public EnergyStorageBase
{
  protected:
    /**
     * The nominal capacity is in the range [0, +infinity).
     */
    J nominalCapacity;

    /**
     * The residual capacity is in the range [0, nominalCapacity].
     */
    J residualCapacity;

    /**
     * Specifies the amount of capacity change which will be reported.
     */
    J printCapacityStep;

    /**
     * The simulation time when the residual capacity was last updated.
     */
    simtime_t lastResidualCapacityUpdate;

    /**
     * The timer that is scheduled to the earliest time when the energy storage
     * will be depleted, the energy storage will be charged, the node will shut
     * down, or the node will start.
     */
    cMessage *timer;

    /**
     * The capacity that will be set when the timer expires.
     */
    J targetCapacity;

    /**
     * When the residual capacity becomes less than this limit the node shuts down.
     */
    J nodeShutdownCapacity;

    /**
     * When the residual capacity becomes more than this limit the node starts.
     */
    J nodeStartCapacity;

    /**
     * The lifecycle controller used to shutdown and start the node.
     */
    LifecycleController *lifecycleController;

    /**
     * The containing node module.
     */
    cModule *node;

    /**
     * The status of the node.
     */
    NodeStatus *nodeStatus;

  public:
    SimpleEnergyStorage();

    virtual ~SimpleEnergyStorage();

    virtual J getNominalCapacity() { return nominalCapacity; }

    virtual J getResidualCapacity() { updateResidualCapacity(); return residualCapacity; }

    virtual void setPowerConsumption(int energyConsumerId, W consumedPower);

    virtual void setPowerGeneration(int energyGeneratorId, W generatedPower);

  protected:
    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *message);

    virtual void executeNodeOperation(J newResidualCapacity);

    virtual void setResidualCapacity(J newResidualCapacity);

    virtual void updateResidualCapacity();

    virtual void scheduleTimer();
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_SIMPLEENERGYSTORAGE_H

