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

#include <algorithm>

#include "inet/power/contract/ICcEnergyStorage.h"
#include "inet/power/contract/IEpEnergyStorage.h"
#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"

namespace inet {
namespace visualizer {

using namespace inet::power;

EnergyStorageVisualizerBase::EnergyStorageVisualization::EnergyStorageVisualization(const IEnergyStorage *energyStorage) :
    energyStorage(energyStorage)
{
}

void EnergyStorageVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayEnergyStorages = par("displayEnergyStorages");
        energyStorageFilter.setPattern(par("energyStorageFilter"));
        color = cFigure::parseColor(par("color"));
        width = par("width");
        height = par("height");
        spacing = par("spacing");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayEnergyStorages)
            addEnergyStorageVisualizations();
    }
}

void EnergyStorageVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        removeAllEnergyStorageVisualizations();
        addEnergyStorageVisualizations();
    }
}

void EnergyStorageVisualizerBase::refreshDisplay() const
{
    for (auto energyStorageVisualization : energyStorageVisualizations)
        refreshEnergyStorageVisualization(energyStorageVisualization);
}

double EnergyStorageVisualizerBase::getNominalCapacity(const IEnergyStorage *energyStorage) const
{
    if (auto epEnergyStorage = dynamic_cast<const IEpEnergyStorage *>(energyStorage))
        return epEnergyStorage->getNominalEnergyCapacity().get();
    else if (auto ccEnergyStorage = dynamic_cast<const ICcEnergyStorage *>(energyStorage))
        return ccEnergyStorage->getNominalChargeCapacity().get();
    else
        throw cRuntimeError("Unknown energy storage");
}

double EnergyStorageVisualizerBase::getResidualCapacity(const IEnergyStorage *energyStorage) const
{
    if (auto epEnergyStorage = dynamic_cast<const IEpEnergyStorage *>(energyStorage))
        return epEnergyStorage->getResidualEnergyCapacity().get();
    else if (auto ccEnergyStorage = dynamic_cast<const ICcEnergyStorage *>(energyStorage))
        return ccEnergyStorage->getResidualChargeCapacity().get();
    else
        throw cRuntimeError("Unknown energy storage");
}

void EnergyStorageVisualizerBase::addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization)
{
    energyStorageVisualizations.push_back(energyStorageVisualization);
}

void EnergyStorageVisualizerBase::removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization)
{
    energyStorageVisualizations.erase(std::remove(energyStorageVisualizations.begin(), energyStorageVisualizations.end(), energyStorageVisualization), energyStorageVisualizations.end());
}

void EnergyStorageVisualizerBase::addEnergyStorageVisualizations()
{
    auto simulation = getSimulation();
    for (int id = 0; id < simulation->getLastComponentId(); id++) {
        auto component = simulation->getComponent(id);
        auto module = dynamic_cast<cModule *>(component);
        auto energyStorage = dynamic_cast<IEnergyStorage *>(module);
        if (energyStorage != nullptr && energyStorageFilter.matches(module))
            addEnergyStorageVisualization(createEnergyStorageVisualization(energyStorage));
    }
}

void EnergyStorageVisualizerBase::removeAllEnergyStorageVisualizations()
{
    for (auto energyStorageVisualization : std::vector<const EnergyStorageVisualization *>(energyStorageVisualizations)) {
        removeEnergyStorageVisualization(energyStorageVisualization);
        delete energyStorageVisualization;
    }
}

} // namespace visualizer
} // namespace inet

