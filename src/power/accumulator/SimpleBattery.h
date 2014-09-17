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

#ifndef __INET_SIMPLEBATTERY_H
#define __INET_SIMPLEBATTERY_H

#include "IPowerAccumulator.h"
#include "PowerSourceBase.h"
#include "PowerSinkBase.h"
#include "LifecycleController.h"
#include "NodeStatus.h"

namespace inet {

namespace power {

/**
 * This class implements a simple battery.
 *
 * @author Levente Meszaros
 */
class INET_API SimpleBattery : public PowerSourceBase, public PowerSinkBase, public virtual IPowerAccumulator
{
  protected:
    /**
     * The nominal capacity of the battery is in the range [0, +infinity).
     */
    J nominalCapacity;

    /**
     * The residual capacity of the battery is in the range [0, nominalCapacity].
     */
    J residualCapacity;

    /**
     * Specifies the amount of capacity change which will be reported.
     */
    J printCapacityStep;

    /**
     * The last simulation time when the residual capacity was updated.
     */
    simtime_t lastResidualCapacityUpdate;

    /**
     * The timer that is scheduled to the time when the battery will be depleted,
     * the battery will be charged, the node will shut down, or the node will
     * start.
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
    SimpleBattery();

    virtual ~SimpleBattery();

    virtual J getNominalCapacity() { return nominalCapacity; }

    virtual J getResidualCapacity() { updateResidualCapacity(); return residualCapacity; }

    virtual void setPowerConsumption(int id, W consumedPower);

    virtual void setPowerGeneration(int id, W generatedPower);

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

#endif // ifndef __INET_SIMPLEBATTERY_H

