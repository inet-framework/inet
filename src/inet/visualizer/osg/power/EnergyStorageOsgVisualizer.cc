//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/power/EnergyStorageOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

using namespace power;

namespace visualizer {

Define_Module(EnergyStorageOsgVisualizer);

EnergyStorageOsgVisualizer::EnergyStorageOsgVisualization::EnergyStorageOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, const IEnergyStorage *energyStorage) :
    EnergyStorageVisualization(energyStorage),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void EnergyStorageOsgVisualizer::initialize(int stage)
{
    EnergyStorageVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
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
    return new EnergyStorageOsgVisualization(networkNodeVisualization, geode, energyStorage);
}

void EnergyStorageOsgVisualizer::refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const
{
    // TODO
//    auto infoOsgVisualization = static_cast<const EnergyStorageOsgVisualization *>(energyStorageVisualization);
//    auto node = infoOsgVisualization->node;
}

} // namespace visualizer

} // namespace inet

