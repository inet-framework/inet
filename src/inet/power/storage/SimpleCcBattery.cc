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

#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"
#include "inet/power/storage/SimpleCcBattery.h"

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
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'c':
                result = getResidualChargeCapacity().str();
                break;
            case 'p':
                result = std::to_string((int)std::round(100 * unit(getResidualChargeCapacity() / getNominalChargeCapacity()).get())) + "%";
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
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
        lifecycleController.initiateOperation(operation);
    }
}

} // namespace power

} // namespace inet

