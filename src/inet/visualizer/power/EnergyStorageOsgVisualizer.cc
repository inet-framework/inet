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
#include "inet/visualizer/power/EnergyStorageOsgVisualizer.h"

namespace inet {

using namespace power;

namespace visualizer {

Define_Module(EnergyStorageOsgVisualizer);

#ifdef WITH_OSG

EnergyStorageOsgVisualizer::EnergyStorageOsgVisualization::EnergyStorageOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, const IEnergyStorage *energyStorage) :
    EnergyStorageVisualization(energyStorage),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

EnergyStorageOsgVisualizer::~EnergyStorageOsgVisualizer()
{
    if (displayEnergyStorages)
        removeAllEnergyStorageVisualizations();
}

void EnergyStorageOsgVisualizer::initialize(int stage)
{
    EnergyStorageVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto energyStorageVisualization : energyStorageVisualizations) {
            auto energyStorageOsgVisualization = static_cast<const EnergyStorageOsgVisualization *>(energyStorageVisualization);
            auto node = energyStorageOsgVisualization->node;
            energyStorageOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

EnergyStorageVisualizerBase::EnergyStorageVisualization *EnergyStorageOsgVisualizer::createEnergyStorageVisualization(const IEnergyStorage *energyStorage) const
{
    auto module = check_and_cast<const cModule *>(energyStorage);
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create energy storage visualization for '%s', because network node visualization is not found for '%s'", module->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new EnergyStorageOsgVisualization(networkNodeVisualization, geode, energyStorage);
}

void EnergyStorageOsgVisualizer::refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const
{
    // TODO:
    // auto infoOsgVisualization = static_cast<const EnergyStorageOsgVisualization *>(energyStorageVisualization);
    // auto node = infoOsgVisualization->node;
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

