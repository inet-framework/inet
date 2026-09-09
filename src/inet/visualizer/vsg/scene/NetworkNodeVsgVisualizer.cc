//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"

#include <vsg/maths/transform.h>

#include <algorithm>
#include <cmath>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeVsgVisualizer);

NetworkNodeVsgVisualizer::~NetworkNodeVsgVisualizer()
{
    for (auto& it : networkNodeVisualizations) {
        auto networkNodeVisualization = it.second;
        while (networkNodeVisualization->getNumAnnotations() != 0)
            networkNodeVisualization->removeAnnotation(0);
    }
}

void NetworkNodeVsgVisualizer::initialize(int stage)
{
    NetworkNodeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayModuleName = par("displayModuleName");
        for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
                auto visualization = createNetworkNodeVisualization(networkNode);
                addNetworkNodeVisualization(visualization);
            }
        }
    }
}

void NetworkNodeVsgVisualizer::refreshDisplay() const
{
    for (auto it : networkNodeVisualizations) {
        auto networkNode = getSimulation()->getModule(it.first);
        auto visualization = it.second;
        auto position = getPosition(networkNode);
        auto orientation = getOrientation(networkNode);
        visualization->setPosition(position);
        double s = std::max(-1.0, std::min(1.0, orientation.s));
        double angle = 2.0 * std::acos(s);
        double sinHalf = std::sqrt(std::max(0.0, 1.0 - s * s));
        ::vsg::dvec3 axis = (sinHalf < 1e-9) ? ::vsg::dvec3(0, 0, 1)
                : ::vsg::dvec3(orientation.v.x / sinHalf, orientation.v.y / sinHalf, orientation.v.z / sinHalf);
        visualization->matrix = ::vsg::translate(::vsg::dvec3(position.x, position.y, position.z)) * ::vsg::rotate(angle, axis);
    }
}

NetworkNodeVsgVisualization *NetworkNodeVsgVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    // returned with refcount 0 (like a fresh osg node); addNetworkNodeVisualization takes ownership
    return new NetworkNodeVsgVisualization(networkNode, displayModuleName);
}

NetworkNodeVsgVisualization *NetworkNodeVsgVisualizer::findNetworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode->getId());
    return it == networkNodeVisualizations.end() ? nullptr : it->second.get();
}

NetworkNodeVsgVisualization *NetworkNodeVsgVisualizer::getNetworkNodeVisualization(const cModule *networkNode) const
{
    return static_cast<NetworkNodeVsgVisualization *>(NetworkNodeVisualizerBase::getNetworkNodeVisualization(networkNode));
}

void NetworkNodeVsgVisualizer::addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeVsgVisualization = check_and_cast<NetworkNodeVsgVisualization *>(networkNodeVisualization);
    networkNodeVisualizations[networkNodeVsgVisualization->networkNode->getId()] = networkNodeVsgVisualization;
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(::vsg::ref_ptr<::vsg::Node>(networkNodeVsgVisualization));
}

void NetworkNodeVsgVisualizer::removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeVsgVisualization = check_and_cast<NetworkNodeVsgVisualization *>(networkNodeVisualization);
    networkNodeVisualizations.erase(networkNodeVisualizations.find(networkNodeVsgVisualization->networkNode->getId()));
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(networkNodeVsgVisualization)), ch.end());
}

} // namespace visualizer

} // namespace inet
