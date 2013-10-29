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

#include "Battery.h"
#include "LifecycleController.h"
#include "NodeOperations.h"
#include "ModuleAccess.h"

Define_Module(Battery);

Battery::Battery()
{
    crashNodeWhenDepleted = false;
    nominalCapacity = 0;
    residualCapacity = 0;
    nominalVoltage = 0;
    internalResistance = 0;
    lastResidualCapacityUpdate = 0;
    depletedTimer = NULL;
}

Battery::~Battery()
{
    cancelAndDelete(depletedTimer);
}

void Battery::initialize(int stage)
{
    if (stage == 0)
    {
        crashNodeWhenDepleted = par("crashNodeWhenDepleted");
        nominalCapacity = residualCapacity = par("nominalCapacity");
        nominalVoltage = par("nominalVoltage");
        internalResistance = par("internalResistance");
        depletedTimer = new cMessage("depleted");
    }
}

void Battery::handleMessage(cMessage * message)
{
    if (message == depletedTimer) {
        updateResidualCapacity();
        emit(powerConsumptionChangedSignal, residualCapacity);
        if (crashNodeWhenDepleted) {
            LifecycleController * lifecycleController = check_and_cast<LifecycleController *>(simulation.getModuleByPath("lifecycleController"));
            NodeCrashOperation * operation = new NodeCrashOperation();
            LifecycleOperation::StringMap params;
            operation->initialize(findContainingNode(this), params);
            lifecycleController->initiateOperation(operation);
        }
    }
}

void Battery::setPowerConsumption(int id, double consumedPower)
{
    Enter_Method_Silent();
    if (residualCapacity == 0)
        throw cRuntimeError("Battery is already depleted");
    else {
        updateResidualCapacity();
        PowerSourceBase::setPowerConsumption(id, consumedPower);
        scheduleDepletedTimer();
    }
}

void Battery::updateResidualCapacity()
{
    simtime_t now = simTime();
    if (now != lastResidualCapacityUpdate) {
        residualCapacity -= (now - lastResidualCapacityUpdate).dbl() * totalConsumedPower;
        lastResidualCapacityUpdate = now;
        ASSERT(residualCapacity >= 0);
    }
}

void Battery::scheduleDepletedTimer()
{
    if (depletedTimer->isScheduled())
        cancelEvent(depletedTimer);
    if (totalConsumedPower > 0)
        scheduleAt(simTime() + residualCapacity / totalConsumedPower, depletedTimer);
}
