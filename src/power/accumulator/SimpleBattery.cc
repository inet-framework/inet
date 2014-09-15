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

#include "SimpleBattery.h"
#include "LifecycleController.h"
#include "NodeOperations.h"
#include "ModuleAccess.h"

namespace inet {

namespace power {

Define_Module(SimpleBattery);

SimpleBattery::SimpleBattery() :
    crashNodeWhenDepleted(false),
    nominalCapacity(J(sNaN)),
    residualCapacity(J(sNaN)),
    lastResidualCapacityUpdate(-1),
    depletedTimer(NULL)
{
}

SimpleBattery::~SimpleBattery()
{
    cancelAndDelete(depletedTimer);
}

void SimpleBattery::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        crashNodeWhenDepleted = par("crashNodeWhenDepleted");
        nominalCapacity = residualCapacity = J(par("nominalCapacity"));
        depletedTimer = new cMessage("depleted");
    }
}

void SimpleBattery::handleMessage(cMessage *message)
{
    if (message == depletedTimer) {
        updateResidualCapacity();
        if (crashNodeWhenDepleted) {
            LifecycleController *lifecycleController = check_and_cast<LifecycleController *>(simulation.getModuleByPath("lifecycleController"));
            NodeCrashOperation *operation = new NodeCrashOperation();
            LifecycleOperation::StringMap params;
            operation->initialize(findContainingNode(this), params);
            lifecycleController->initiateOperation(operation);
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimpleBattery::setPowerConsumption(int id, W consumedPower)
{
    Enter_Method_Silent();
    if (residualCapacity == J(0))
        throw cRuntimeError("Battery is already depleted");
    else {
        updateResidualCapacity();
        PowerSourceBase::setPowerConsumption(id, consumedPower);
        scheduleDepletedTimer();
    }
}

void SimpleBattery::updateResidualCapacity()
{
    simtime_t now = simTime();
    if (now != lastResidualCapacityUpdate) {
        residualCapacity -= s((now - lastResidualCapacityUpdate).dbl()) * totalConsumedPower;
        lastResidualCapacityUpdate = now;
        emit(residualCapacityChangedSignal, residualCapacity.get());
        ASSERT(residualCapacity >= J(0));
    }
}

void SimpleBattery::scheduleDepletedTimer()
{
    if (depletedTimer->isScheduled())
        cancelEvent(depletedTimer);
    if (totalConsumedPower > W(0))
        scheduleAt(simTime() + unit(residualCapacity / totalConsumedPower / s(1.0)).get(), depletedTimer);
}

} // namespace power

} // namespace inet

