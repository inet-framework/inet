//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/storage/SimpleCcBattery.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"

namespace inet {

namespace power {

Define_Module(SimpleCcBattery);

void SimpleCcBattery::initialize(int stage)
{
    CcEnergyStorageBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        nominalCapacity = C(par("nominalCapacity"));
        nominalVoltage = V(par("nominalVoltage"));
        internalResistance = Ohm(par("internalResistance"));
        networkNode = findContainingNode(this);
        if (networkNode != nullptr) {
            nodeStatus = dynamic_cast<NodeStatus *>(networkNode->getSubmodule("status"));
            if (!nodeStatus)
                throw cRuntimeError("Cannot find node status");
        }
        setResidualCapacity(C(par("initialCapacity")));
        WATCH(residualCapacity);
    }
}

void SimpleCcBattery::refreshDisplay() const
{
    updateDisplayString();
}

void SimpleCcBattery::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string SimpleCcBattery::resolveDirective(char directive) const
{
    switch (directive) {
        case 'c':
            return getResidualChargeCapacity().str();
        case 'p':
            return std::to_string((int)std::round(100 * unit(getResidualChargeCapacity() / getNominalChargeCapacity()).get())) + "%";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void SimpleCcBattery::updateTotalCurrentConsumption()
{
    updateResidualCapacity();
    CcEnergyStorageBase::updateTotalCurrentConsumption();
}

void SimpleCcBattery::updateTotalCurrentGeneration()
{
    updateResidualCapacity();
    CcEnergyStorageBase::updateTotalCurrentGeneration();
}

void SimpleCcBattery::setResidualCapacity(C newResidualCapacity)
{
    residualCapacity = newResidualCapacity;
    lastResidualCapacityUpdate = simTime();
    if (residualCapacity == C(0))
        EV_WARN << "Battery depleted" << endl;
    else if (residualCapacity == nominalCapacity)
        EV_INFO << "Battery charged" << endl;
    if (networkNode != nullptr)
        executeNodeOperation(newResidualCapacity);
    emit(residualChargeCapacityChangedSignal, residualCapacity.get());
}

void SimpleCcBattery::updateResidualCapacity()
{
    simtime_t currentTime = simTime();
    if (currentTime != lastResidualCapacityUpdate) {
        C newResidualCapacity = residualCapacity + s((currentTime - lastResidualCapacityUpdate).dbl()) * (totalCurrentGeneration - totalCurrentConsumption);
        if (newResidualCapacity < C(0))
            newResidualCapacity = C(0);
        else if (newResidualCapacity > nominalCapacity)
            newResidualCapacity = nominalCapacity;
        setResidualCapacity(newResidualCapacity);
    }
}

void SimpleCcBattery::executeNodeOperation(C newResidualCapacity)
{
    if (nodeStatus->getState() == NodeStatus::UP && newResidualCapacity <= C(0)) {
        EV_WARN << "Battery failed" << endl;
        LifecycleOperation::StringMap params;
        ModuleCrashOperation *operation = new ModuleCrashOperation();
        operation->initialize(networkNode, params);
        initiateOperation(operation);
    }
}

} // namespace power

} // namespace inet

