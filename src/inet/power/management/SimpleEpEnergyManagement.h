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

#ifndef __INET_SIMPLEEPENERGYMANAGEMENT_H
#define __INET_SIMPLEEPENERGYMANAGEMENT_H

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/power/contract/IEpEnergyManagement.h"
#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {

namespace power {

class INET_API SimpleEpEnergyManagement : public cSimpleModule, public virtual IEpEnergyManagement, public cListener
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

    LifecycleController lifecycleController;

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

#endif // ifndef __INET_SIMPLEEPENERGYMANAGEMENT_H

