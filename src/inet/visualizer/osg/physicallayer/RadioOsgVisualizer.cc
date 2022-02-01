//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/physicallayer/RadioOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(RadioOsgVisualizer);

RadioOsgVisualizer::RadioOsgVisualization::RadioOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, const int radioModuleId) :
    RadioVisualization(radioModuleId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void RadioOsgVisualizer::initialize(int stage)
{
    RadioVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto it : radioVisualizations) {
            auto radioOsgVisualization = static_cast<const RadioOsgVisualization *>(it.second);
            auto node = radioOsgVisualization->node;
            radioOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

RadioVisualizerBase::RadioVisualization *RadioOsgVisualizer::createRadioVisualization(const IRadio *radio) const
{
    auto module = check_and_cast<const cModule *>(radio);
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new RadioOsgVisualization(networkNodeVisualization, geode, module->getId());
}

void RadioOsgVisualizer::refreshRadioVisualization(const RadioVisualization *radioVisualization) const
{
    // TODO
//    auto infoOsgVisualization = static_cast<const RadioOsgVisualization *>(radioVisualization);
//    auto node = infoOsgVisualization->node;
}

} // namespace visualizer

} // namespace inet

