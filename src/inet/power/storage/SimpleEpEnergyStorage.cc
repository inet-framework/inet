//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/storage/SimpleEpEnergyStorage.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"

namespace inet {

namespace power {

Define_Module(SimpleEpEnergyStorage);

SimpleEpEnergyStorage::~SimpleEpEnergyStorage()
{
    cancelAndDelete(timer);
}

void SimpleEpEnergyStorage::initialize(int stage)
{
    EpEnergyStorageBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        nominalCapacity = J(par("nominalCapacity"));
        printCapacityStep = J(par("printCapacityStep"));
        timer = new cMessage("timer");
        networkNode = findContainingNode(this);
        if (networkNode != nullptr) {
            nodeStatus = dynamic_cast<NodeStatus *>(networkNode->getSubmodule("status"));
            if (!nodeStatus)
                throw cRuntimeError("Cannot find node status");
        }
        setResidualCapacity(J(par("initialCapacity")));
        scheduleTimer();
        WATCH(residualCapacity);
    }
}

void SimpleEpEnergyStorage::handleMessage(cMessage *message)
{
    if (message == timer) {
        setResidualCapacity(targetCapacity);
        scheduleTimer();
        EV_INFO << "Residual capacity = " << residualCapacity.get() << " (" << (int)round(unit(residualCapacity / nominalCapacity).get() * 100) << "%)" << endl;
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimpleEpEnergyStorage::refreshDisplay() const
{
    updateDisplayString();
}

void SimpleEpEnergyStorage::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string SimpleEpEnergyStorage::resolveDirective(char directive) const
{
    switch (directive) {
        case 'c':
            return getResidualEnergyCapacity().str();
        case 'p':
            return std::to_string((int)std::round(100 * unit(getResidualEnergyCapacity() / getNominalEnergyCapacity()).get())) + "%";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void SimpleEpEnergyStorage::updateTotalPowerConsumption()
{
    updateResidualCapacity();
    EpEnergyStorageBase::updateTotalPowerConsumption();
    scheduleTimer();
}

void SimpleEpEnergyStorage::updateTotalPowerGeneration()
{
    updateResidualCapacity();
    EpEnergyStorageBase::updateTotalPowerGeneration();
    scheduleTimer();
}

void SimpleEpEnergyStorage::executeNodeOperation(J newResidualCapacity)
{
    if (newResidualCapacity <= J(0) && nodeStatus->getState() == NodeStatus::UP) {
        EV_WARN << "Energy storage failed" << endl;
        LifecycleOperation::StringMap params;
        ModuleCrashOperation *operation = new ModuleCrashOperation();
        operation->initialize(networkNode, params);
        initiateOperation(operation);
    }
}

J SimpleEpEnergyStorage::getResidualEnergyCapacity() const
{
    const_cast<SimpleEpEnergyStorage *>(this)->updateResidualCapacity();
    return residualCapacity;
}

void SimpleEpEnergyStorage::setResidualCapacity(J newResidualCapacity)
{
    residualCapacity = newResidualCapacity;
    lastResidualCapacityUpdate = simTime();
    if (residualCapacity == J(0))
        EV_WARN << "Energy storage depleted" << endl;
    else if (residualCapacity == nominalCapacity)
        EV_INFO << "Energy storage charged" << endl;
    if (networkNode != nullptr)
        executeNodeOperation(newResidualCapacity);
    emit(residualEnergyCapacityChangedSignal, residualCapacity.get());
}

void SimpleEpEnergyStorage::updateResidualCapacity()
{
    simtime_t currentSimulationTime = simTime();
    if (currentSimulationTime != lastResidualCapacityUpdate) {
        J newResidualCapacity = residualCapacity + s((currentSimulationTime - lastResidualCapacityUpdate).dbl()) * (totalPowerGeneration - totalPowerConsumption);
        if (newResidualCapacity < J(0))
            newResidualCapacity = J(0);
        else if (newResidualCapacity > nominalCapacity)
            newResidualCapacity = nominalCapacity;
        setResidualCapacity(newResidualCapacity);
    }
}

void SimpleEpEnergyStorage::scheduleTimer()
{
    W totalPower = totalPowerGeneration - totalPowerConsumption;
    targetCapacity = residualCapacity;
    if (totalPower > W(0)) {
        targetCapacity = std::isnan(printCapacityStep.get()) ? nominalCapacity : ceil(unit(residualCapacity / printCapacityStep).get()) * printCapacityStep;
        // NOTE: make sure capacity will change over time despite double arithmetic
        simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
        if (remainingTime == 0)
            targetCapacity += printCapacityStep;
    }
    else if (totalPower < W(0)) {
        targetCapacity = std::isnan(printCapacityStep.get()) ? J(0) : floor(unit(residualCapacity / printCapacityStep).get()) * printCapacityStep;
        // make sure capacity will change over time despite double arithmetic
        simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
        if (remainingTime == 0)
            targetCapacity -= printCapacityStep;
    }
    // enforce target capacity to be in range
    if (targetCapacity < J(0))
        targetCapacity = J(0);
    else if (targetCapacity > nominalCapacity)
        targetCapacity = nominalCapacity;
    if (timer->isScheduled())
        cancelEvent(timer);
    // don't schedule if there's no progress
    if (totalPower != W(0)) {
        simtime_t remainingTime = unit((targetCapacity - residualCapacity) / totalPower / s(1)).get();
        if (remainingTime > 0)
            scheduleAfter(remainingTime, timer);
    }
}

} // namespace power

} // namespace inet

