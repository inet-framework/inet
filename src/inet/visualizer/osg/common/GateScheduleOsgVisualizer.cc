//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/common/GateScheduleOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(GateScheduleOsgVisualizer);

GateScheduleOsgVisualizer::GateOsgVisualization::GateOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, queueing::IPacketGate *gate) :
    GateVisualization(gate),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void GateScheduleOsgVisualizer::initialize(int stage)
{
    GateScheduleVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto gateVisualization : gateVisualizations) {
            auto gateOsgVisualization = static_cast<const GateOsgVisualization *>(gateVisualization);
            auto node = gateOsgVisualization->node;
            gateOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

GateScheduleVisualizerBase::GateVisualization *GateScheduleOsgVisualizer::createGateVisualization(queueing::IPacketGate *gate) const
{
    auto ownedObject = check_and_cast<cOwnedObject *>(gate);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new GateOsgVisualization(networkNodeVisualization, geode, gate);
}

void GateScheduleOsgVisualizer::refreshGateVisualization(const GateVisualization *gateVisualization) const
{
    // TODO
//    auto infoOsgVisualization = static_cast<const GateOsgVisualization *>(gateVisualization);
//    auto node = infoOsgVisualization->node;
}

} // namespace visualizer

} // namespace inet

