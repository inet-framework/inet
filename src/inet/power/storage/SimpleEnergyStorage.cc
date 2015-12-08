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

#include "inet/power/storage/SimpleEnergyStorage.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace power {

Define_Module(SimpleEnergyStorage);

SimpleEnergyStorage::SimpleEnergyStorage() :
    nominalCapacity(J(NaN)),
    residualCapacity(J(NaN)),
    printCapacityStep(J(NaN)),
    lastResidualCapacityUpdate(-1),
    timer(nullptr),
    nodeShutdownCapacity(J(NaN)),
    nodeStartCapacity(J(NaN)),
    lifecycleController(nullptr),
    node(nullptr),
    nodeStatus(nullptr)
{
}

SimpleEnergyStorage::~SimpleEnergyStorage()
{
    cancelAndDelete(timer);
}

void SimpleEnergyStorage::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        nominalCapacity = J(par("nominalCapacity"));
        residualCapacity = J(par("initialCapacity"));
        printCapacityStep = J(par("printCapacityStep"));
        nodeShutdownCapacity = J(par("nodeShutdownCapacity"));
        nodeStartCapacity = J(par("nodeStartCapacity"));
        lastResidualCapacityUpdate = simTime();
        emit(residualCapacityChangedSignal, residualCapacity.get());
        timer = new cMessage("timer");
        if (!std::isnan(nodeStartCapacity.get()) || !std::isnan(nodeShutdownCapacity.get())) {
            node = findContainingNode(this);
            nodeStatus = dynamic_cast<NodeStatus *>(node->getSubmodule("status"));
            if (!nodeStatus)
                throw cRuntimeError("Cannot find node status");
            lifecycleController = dynamic_cast<LifecycleController *>(getModuleByPath("lifecycleController"));
            if (!lifecycleController)
                throw cRuntimeError("Cannot find lifecycle controller");
        }
        scheduleTimer();
        WATCH(residualCapacity);
    }
}

void SimpleEnergyStorage::handleMessage(cMessage *message)
{
    if (message == timer) {
        setResidualCapacity(targetCapacity);
        scheduleTimer();
        EV_INFO << "Residual capacity = " << residualCapacity.get() << " (" << (int)round(unit(residualCapacity / nominalCapacity).get() * 100) << "%)" << endl;
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimpleEnergyStorage::setPowerConsumption(int energyConsumerId, W consumedPower)
{
    Enter_Method_Silent();
    updateResidualCapacity();
    EnergySourceBase::setPowerConsumption(energyConsumerId, consumedPower);
    scheduleTimer();
}

void SimpleEnergyStorage::setPowerGeneration(int energyGeneratorId, W generatedPower)
{
    Enter_Method_Silent();
    updateResidualCapacity();
    EnergySinkBase::setPowerGeneration(energyGeneratorId, generatedPower);
    scheduleTimer();
}

void SimpleEnergyStorage::executeNodeOperation(J newResidualCapacity)
{
    if (!std::isnan(nodeShutdownCapacity.get()) && newResidualCapacity <= nodeShutdownCapacity && nodeStatus->getState() == NodeStatus::UP) {
        EV_WARN << "Capacity reached node shutdown threshold" << endl;
        LifecycleOperation::StringMap params;
        NodeShutdownOperation *operation = new NodeShutdownOperation();
        operation->initialize(node, params);
        lifecycleController->initiateOperation(operation);
    }
    else if (!std::isnan(nodeStartCapacity.get()) && newResidualCapacity >= nodeStartCapacity && nodeStatus->getState() == NodeStatus::DOWN) {
        EV_INFO << "Capacity reached node start threshold" << endl;
        LifecycleOperation::StringMap params;
        NodeStartOperation *operation = new NodeStartOperation();
        operation->initialize(node, params);
        lifecycleController->initiateOperation(operation);
    }
}

void SimpleEnergyStorage::setResidualCapacity(J newResidualCapacity)
{
    if (newResidualCapacity != residualCapacity) {
        residualCapacity = newResidualCapacity;
        lastResidualCapacityUpdate = simTime();
        executeNodeOperation(newResidualCapacity);
        if (residualCapacity == J(0))
            EV_WARN << "Energy storage depleted" << endl;
        else if (residualCapacity == nominalCapacity)
            EV_INFO << "Energy storage charged" << endl;
        emit(residualCapacityChangedSignal, residualCapacity.get());
    }
}

void SimpleEnergyStorage::updateResidualCapacity()
{
    simtime_t now = simTime();
    if (now != lastResidualCapacityUpdate) {
        J newResidualCapacity = residualCapacity + s((now - lastResidualCapacityUpdate).dbl()) * (totalGeneratedPower - totalConsumedPower);
        if (newResidualCapacity < J(0))
            newResidualCapacity = J(0);
        else if (newResidualCapacity > nominalCapacity)
            newResidualCapacity = nominalCapacity;
        setResidualCapacity(newResidualCapacity);
    }
}

void SimpleEnergyStorage::scheduleTimer()
{
    W totalPower = totalGeneratedPower - totalConsumedPower;
    targetCapacity = residualCapacity;
    if (totalPower > W(0)) {
        targetCapacity = std::isnan(printCapacityStep.get()) ? nominalCapacity : ceil(unit(residualCapacity / printCapacityStep).get()) * printCapacityStep;
        // NOTE: make sure capacity will change over time despite double arithmetic
        simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
        if (remainingTime == 0)
            targetCapacity += printCapacityStep;
        // override target capacity if start is needed
        if (!std::isnan(nodeStartCapacity.get()) && nodeStatus->getState() == NodeStatus::DOWN && residualCapacity < nodeStartCapacity && nodeStartCapacity < targetCapacity)
            targetCapacity = nodeStartCapacity;
    }
    else if (totalPower < W(0)) {
        targetCapacity = std::isnan(printCapacityStep.get()) ? J(0) : floor(unit(residualCapacity / printCapacityStep).get()) * printCapacityStep;
        // make sure capacity will change over time despite double arithmetic
        simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
        if (remainingTime == 0)
            targetCapacity -= printCapacityStep;
        // override target capacity if shutdown is needed
        if (!std::isnan(nodeShutdownCapacity.get()) && nodeStatus->getState() == NodeStatus::UP && residualCapacity > nodeShutdownCapacity && nodeShutdownCapacity > targetCapacity)
            targetCapacity = nodeShutdownCapacity;
    }
    // enforce target capacity to be in range
    if (targetCapacity < J(0))
        targetCapacity = J(0);
    else if (targetCapacity > nominalCapacity)
        targetCapacity = nominalCapacity;
    simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
    if (timer->isScheduled())
        cancelEvent(timer);
    // don't schedule if there's no progress
    if (remainingTime > 0)
        scheduleAt(simTime() + remainingTime, timer);
}

} // namespace power

} // namespace inet

