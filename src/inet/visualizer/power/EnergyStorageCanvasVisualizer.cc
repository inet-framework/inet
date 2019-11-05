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
#include "inet/visualizer/power/EnergyStorageCanvasVisualizer.h"

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

EnergyStorageCanvasVisualizer::~EnergyStorageCanvasVisualizer()
{
    if (displayEnergyStorages)
        removeAllEnergyStorageVisualizations();
}

void EnergyStorageCanvasVisualizer::initialize(int stage)
{
    EnergyStorageVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
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
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create energy storage visualization for '%s', because network node visualization is not found for '%s'", module->getFullPath().c_str(), networkNode->getFullPath().c_str());
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

