//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"

#include <omnetpp/osgutil.h>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeOsgVisualizer);

NetworkNodeOsgVisualizer::~NetworkNodeOsgVisualizer()
{
    for (auto& it : networkNodeVisualizations) {
        auto networkNodeVisualization = it.second;
        while (networkNodeVisualization->getNumAnnotations() != 0)
            networkNodeVisualization->removeAnnotation(0);
    }
}

void NetworkNodeOsgVisualizer::initialize(int stage)
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

void NetworkNodeOsgVisualizer::refreshDisplay() const
{
    for (auto it : networkNodeVisualizations) {
        auto networkNode = getSimulation()->getModule(it.first);
        auto visualization = it.second;
        auto position = getPosition(networkNode);
        auto orientation = getOrientation(networkNode);
        visualization->setPosition(osg::Vec3d(position.x, position.y, position.z));
        visualization->setAttitude(osg::Quat(osg::Vec4d(orientation.v.x, orientation.v.y, orientation.v.z, orientation.s)));
    }
}

NetworkNodeOsgVisualization *NetworkNodeOsgVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    return new NetworkNodeOsgVisualization(networkNode, displayModuleName);
}

NetworkNodeOsgVisualization *NetworkNodeOsgVisualizer::findNetworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode->getId());
    return it == networkNodeVisualizations.end() ? nullptr : it->second;
}

NetworkNodeOsgVisualization *NetworkNodeOsgVisualizer::getNetworkNodeVisualization(const cModule *networkNode) const
{
    return static_cast<NetworkNodeOsgVisualization *>(NetworkNodeVisualizerBase::getNetworkNodeVisualization(networkNode));
}

void NetworkNodeOsgVisualizer::addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeOsgVisualization = check_and_cast<NetworkNodeOsgVisualization *>(networkNodeVisualization);
    networkNodeVisualizations[networkNodeOsgVisualization->networkNode->getId()] = networkNodeOsgVisualization;
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(networkNodeOsgVisualization);
}

void NetworkNodeOsgVisualizer::removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeOsgVisualization = check_and_cast<NetworkNodeOsgVisualization *>(networkNodeVisualization);
    networkNodeVisualizations.erase(networkNodeVisualizations.find(networkNodeOsgVisualization->networkNode->getId()));
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->removeChild(networkNodeOsgVisualization);
}

} // namespace visualizer

} // namespace inet

