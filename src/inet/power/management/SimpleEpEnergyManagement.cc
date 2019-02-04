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

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/power/management/SimpleEpEnergyManagement.h"

namespace inet {

namespace power {

Define_Module(SimpleEpEnergyManagement);

SimpleEpEnergyManagement::~SimpleEpEnergyManagement()
{
    cancelAndDelete(lifecycleOperationTimer);
}

void SimpleEpEnergyManagement::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        nodeShutdownCapacity = J(par("nodeShutdownCapacity"));
        nodeStartCapacity = J(par("nodeStartCapacity"));
        networkNode = findContainingNode(this);
        energyStorage = check_and_cast<IEpEnergyStorage *>(networkNode->getSubmodule("energyStorage"));
        auto energyStorageModule = check_and_cast<cModule *>(energyStorage);
        energyStorageModule->subscribe(IEpEnergySource::powerConsumptionChangedSignal, this);
        energyStorageModule->subscribe(IEpEnergySink::powerGenerationChangedSignal, this);
        nodeStatus = dynamic_cast<NodeStatus *>(networkNode->getSubmodule("status"));
        if (!nodeStatus)
            throw cRuntimeError("Cannot find node status");
        lifecycleOperationTimer = new cMessage("lifecycleOperation");
    }
}

void SimpleEpEnergyManagement::handleMessage(cMessage *message)
{
    if (message == lifecycleOperationTimer) {
        executeNodeOperation(getEstimatedEnergyCapacity());
        scheduleLifecycleOperationTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimpleEpEnergyManagement::refreshDisplay() const
{
    std::string text;
    if (std::isnan(targetCapacity.get()))
        text = "";
    else if (targetCapacity == nodeShutdownCapacity)
        text = "shutdown";
    else if (targetCapacity == nodeStartCapacity)
        text = "start";
    else
        throw cRuntimeError("Invalid state");
    if (text.length() != 0)
        text += " in " + (lifecycleOperationTimer->getArrivalTime() - simTime()).str() + " s";
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void SimpleEpEnergyManagement::executeNodeOperation(J estimatedEnergyCapacity)
{
    if (!std::isnan(nodeShutdownCapacity.get()) && estimatedEnergyCapacity <= nodeShutdownCapacity && nodeStatus->getState() == NodeStatus::UP) {
        EV_WARN << "Capacity reached node shutdown threshold" << endl;
        LifecycleOperation::StringMap params;
        ModuleStopOperation *operation = new ModuleStopOperation();
        operation->initialize(networkNode, params);
        lifecycleController.initiateOperation(operation);
    }
    else if (!std::isnan(nodeStartCapacity.get()) && estimatedEnergyCapacity >= nodeStartCapacity && nodeStatus->getState() == NodeStatus::DOWN) {
        EV_INFO << "Capacity reached node start threshold" << endl;
        LifecycleOperation::StringMap params;
        ModuleStartOperation *operation = new ModuleStartOperation();
        operation->initialize(networkNode, params);
        lifecycleController.initiateOperation(operation);
    }
}

void SimpleEpEnergyManagement::scheduleLifecycleOperationTimer()
{
    targetCapacity = J(NaN);
    J estimatedResidualCapacity = getEstimatedEnergyCapacity();
    W totalPower = energyStorage->getTotalPowerGeneration() - energyStorage->getTotalPowerConsumption();
    if (totalPower > W(0)) {
        // override target capacity if start is needed
        if (!std::isnan(nodeStartCapacity.get()) && nodeStatus->getState() == NodeStatus::DOWN && estimatedResidualCapacity < nodeStartCapacity)
            targetCapacity = nodeStartCapacity;
    }
    else if (totalPower < W(0)) {
        // override target capacity if shutdown is needed
        if (!std::isnan(nodeShutdownCapacity.get()) && nodeStatus->getState() == NodeStatus::UP && estimatedResidualCapacity > nodeShutdownCapacity)
            targetCapacity = nodeShutdownCapacity;
    }
    // enforce target capacity to be in range
    simtime_t remainingTime = unit((targetCapacity - estimatedResidualCapacity) / totalPower / s(1)).get();
    // make sure the targetCapacity is reached despite floating point arithmetic
    remainingTime.setRaw(remainingTime.raw() + 1);
    if (lifecycleOperationTimer->isScheduled())
        cancelEvent(lifecycleOperationTimer);
    // don't schedule if there's no progress
    if (remainingTime > 0)
        scheduleAt(simTime() + remainingTime, lifecycleOperationTimer);
}

J SimpleEpEnergyManagement::getEstimatedEnergyCapacity() const
{
    // NOTE: cheat with estimation and return the actual energy capacity known by the energy storage
    return energyStorage->getResidualEnergyCapacity();
}

void SimpleEpEnergyManagement::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method_Silent("receiveSignal");
    if (signal == IEpEnergySource::powerConsumptionChangedSignal || signal == IEpEnergySink::powerGenerationChangedSignal) {
        executeNodeOperation(getEstimatedEnergyCapacity());
        scheduleLifecycleOperationTimer();
    }
}

} // namespace power

} // namespace inet

