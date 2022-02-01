//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/power/EnergyStorageCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::power;

Define_Module(EnergyStorageCanvasVisualizer);

EnergyStorageCanvasVisualizer::EnergyStorageCanvasVisualization::EnergyStorageCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BarFigure *figure, const IEnergyStorage *energyStorage) :
    EnergyStorageVisualization(energyStorage),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

EnergyStorageCanvasVisualizer::EnergyStorageCanvasVisualization::~EnergyStorageCanvasVisualization()
{
    delete figure;
}

void EnergyStorageCanvasVisualizer::initialize(int stage)
{
    EnergyStorageVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

EnergyStorageVisualizerBase::EnergyStorageVisualization *EnergyStorageCanvasVisualizer::createEnergyStorageVisualization(const IEnergyStorage *energyStorage) const
{
    auto module = check_and_cast<const cModule *>(energyStorage);
    auto figure = new BarFigure("energyStorage");
    figure->setTags((std::string("energyStorage ") + tags).c_str());
    figure->setTooltip("This figure represents an energy storage");
    figure->setAssociatedObject(const_cast<cModule *>(module));
    figure->setBounds(cFigure::Rectangle(0, 0, width, height));
    figure->setSpacing(spacing);
    figure->setColor(color);
    figure->setMinValue(0);
    figure->setMaxValue(getNominalCapacity(energyStorage));
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new EnergyStorageCanvasVisualization(networkNodeVisualization, figure, energyStorage);
}

void EnergyStorageCanvasVisualizer::addEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization)
{
    EnergyStorageVisualizerBase::addEnergyStorageVisualization(energyStorageVisualization);
    auto energyStorageCanvasVisualization = static_cast<const EnergyStorageCanvasVisualization *>(energyStorageVisualization);
    auto figure = energyStorageCanvasVisualization->figure;
    energyStorageCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize(), placementHint, placementPriority);
}

void EnergyStorageCanvasVisualizer::removeEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization)
{
    EnergyStorageVisualizerBase::removeEnergyStorageVisualization(energyStorageVisualization);
    auto energyStorageCanvasVisualization = static_cast<const EnergyStorageCanvasVisualization *>(energyStorageVisualization);
    auto figure = energyStorageCanvasVisualization->figure;
    if (networkNodeVisualizer != nullptr)
        energyStorageCanvasVisualization->networkNodeVisualization->removeAnnotation(figure);
}

void EnergyStorageCanvasVisualizer::refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const
{
    auto energyStorageCanvasVisualization = static_cast<const EnergyStorageCanvasVisualization *>(energyStorageVisualization);
    auto energyStorage = energyStorageVisualization->energyStorage;
    auto figure = energyStorageCanvasVisualization->figure;
    figure->setValue(getResidualCapacity(energyStorage));
}

} // namespace visualizer

} // namespace inet

