//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/networklayer/RoutingTableOsgVisualizer.h"

#include <osg/Geode>
#include <osg/LineWidth>

#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(RoutingTableOsgVisualizer);

RoutingTableOsgVisualizer::RouteOsgVisualization::RouteOsgVisualization(osg::Node *node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
    RouteVisualization(route, nodeModuleId, nextHopModuleId),
    node(node)
{
}

void RoutingTableOsgVisualizer::initialize(int stage)
{
    RoutingTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getOsgLineManager(canvas);
    }
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableOsgVisualizer::createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const
{
    auto nodePosition = getPosition(node);
    auto nextHopPosition = getPosition(nextHop);
    auto osgNode = inet::osg::createLine(nodePosition, nextHopPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    auto stateSet = inet::osg::createStateSet(lineColor, 1.0, false);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    osgNode->setStateSet(stateSet);
    return new RouteOsgVisualization(osgNode, route, node->getId(), nextHop->getId());
}

void RoutingTableOsgVisualizer::addRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::addRouteVisualization(routeVisualization);
    auto routeOsgVisualization = static_cast<const RouteOsgVisualization *>(routeVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(routeOsgVisualization->node);
}

void RoutingTableOsgVisualizer::removeRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::removeRouteVisualization(routeVisualization);
    auto routeOsgVisualization = static_cast<const RouteOsgVisualization *>(routeVisualization);
    auto node = routeOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void RoutingTableOsgVisualizer::refreshRouteVisualization(const RouteVisualization *routeVisualization) const
{
    // TODO
//    auto routeOsgVisualization = static_cast<const RouteOsgVisualization *>(routeVisualization);
//    auto text = getRouteVisualizationText(routeVisualization->route);
}

} // namespace visualizer

} // namespace inet

