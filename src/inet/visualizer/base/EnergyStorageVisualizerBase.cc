//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/EnergyStorageVisualizerBase.h"

#include <algorithm>

#include "inet/power/contract/ICcEnergyStorage.h"
#include "inet/power/contract/IEpEnergyStorage.h"

namespace inet {
namespace visualizer {

using namespace inet::power;

EnergyStorageVisualizerBase::EnergyStorageVisualization::EnergyStorageVisualization(const IEnergyStorage *energyStorage) :
    energyStorage(energyStorage)
{
}

void EnergyStorageVisualizerBase::preDelete(cComponent *root)
{
    if (displayEnergyStorages)
        removeAllEnergyStorageVisualizations();
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
    removeAllEnergyStorageVisualizations();
    addEnergyStorageVisualizations();
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

