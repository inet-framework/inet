//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeCanvasVisualizer);

NetworkNodeCanvasVisualizer::~NetworkNodeCanvasVisualizer()
{
    for (auto& it : networkNodeVisualizations) {
        auto networkNodeVisualization = it.second;
        while (networkNodeVisualization->getNumAnnotations() != 0)
            networkNodeVisualization->removeAnnotation(0);
    }
}

void NetworkNodeCanvasVisualizer::initialize(int stage)
{
    NetworkNodeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
                auto visualization = createNetworkNodeVisualization(networkNode);
                addNetworkNodeVisualization(visualization);
            }
        }
    }
}

void NetworkNodeCanvasVisualizer::refreshDisplay() const
{
    for (auto it : networkNodeVisualizations) {
        auto networkNode = getSimulation()->getModule(it.first);
        auto visualization = it.second;
        auto position = canvasProjection->computeCanvasPoint(getPosition(networkNode));
        visualization->setTransform(cFigure::Transform().translate(position.x, position.y));
        visualization->refreshDisplay();
    }
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    auto visualization = new NetworkNodeCanvasVisualization(networkNode, annotationSpacing, placementPenalty);
    visualization->setZIndex(zIndex);
    return visualization;
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::findNetworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode->getId());
    return it == networkNodeVisualizations.end() ? nullptr : it->second;
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::getNetworkNodeVisualization(const cModule *networkNode) const
{
    return static_cast<NetworkNodeCanvasVisualization *>(NetworkNodeVisualizerBase::getNetworkNodeVisualization(networkNode));
}

void NetworkNodeCanvasVisualizer::addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeCanvasVisualization = check_and_cast<NetworkNodeCanvasVisualization *>(networkNodeVisualization);
    networkNodeVisualizations[networkNodeCanvasVisualization->networkNode->getId()] = networkNodeCanvasVisualization;
    visualizationTargetModule->getCanvas()->addFigure(networkNodeCanvasVisualization);
}

void NetworkNodeCanvasVisualizer::removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeCanvasVisualization = check_and_cast<NetworkNodeCanvasVisualization *>(networkNodeVisualization);
    networkNodeVisualizations.erase(networkNodeVisualizations.find(networkNodeCanvasVisualization->networkNode->getId()));
    visualizationTargetModule->getCanvas()->removeFigure(networkNodeCanvasVisualization);
}

} // namespace visualizer

} // namespace inet

