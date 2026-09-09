//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/networklayer/RoutingTableVsgVisualizer.h"

#include <algorithm>

#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(RoutingTableVsgVisualizer);

RoutingTableVsgVisualizer::RouteVsgVisualization::RouteVsgVisualization(::vsg::ref_ptr<::vsg::Node> node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
    RouteVisualization(route, nodeModuleId, nextHopModuleId),
    node(node)
{
}

RoutingTableVsgVisualizer::MulticastRouteVsgVisualization::MulticastRouteVsgVisualization(::vsg::ref_ptr<::vsg::Node> node, const Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId) :
    MulticastRouteVisualization(route, nodeModuleId, nextHopModuleId),
    node(node)
{
}

void RoutingTableVsgVisualizer::initialize(int stage)
{
    RoutingTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getVsgLineManager(canvas);
    }
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableVsgVisualizer::createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const
{
    auto nodePosition = getPosition(node);
    auto nextHopPosition = getPosition(nextHop);
    // OSG used osg::LineWidth + createStateSet; VSG bakes width/color into the line geometry.
    auto vsgNode = inet::vsg::createLine(nodePosition, nextHopPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED,
            lineColor, lineStyle, lineWidth);
    return new RouteVsgVisualization(vsgNode, route, node->getId(), nextHop->getId());
}

void RoutingTableVsgVisualizer::addRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::addRouteVisualization(routeVisualization);
    auto routeVsgVisualization = static_cast<const RouteVsgVisualization *>(routeVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(routeVsgVisualization->node);
}

void RoutingTableVsgVisualizer::removeRouteVisualization(const RouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::removeRouteVisualization(routeVisualization);
    auto routeVsgVisualization = static_cast<const RouteVsgVisualization *>(routeVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(routeVsgVisualization->node)), ch.end());
}

void RoutingTableVsgVisualizer::refreshRouteVisualization(const RouteVisualization *routeVisualization) const
{
    // TODO: rebuild line label text when displayLabels is true, e.g.:
    //   auto text = getRouteVisualizationText(routeVisualization->route);
    // VSG lines have no detachable label child; adding one would require storing
    // a vsg::Group container (like PathVsgVisualization::node) so the label node
    // can be replaced.
}

const RoutingTableVisualizerBase::MulticastRouteVisualization *RoutingTableVsgVisualizer::createMulticastRouteVisualization(Ipv4MulticastRoute *route, cModule *node, cModule *nextHop) const
{
    auto nodePosition = getPosition(node);
    auto nextHopPosition = getPosition(nextHop);
    // OSG used osg::LineWidth + createStateSet; VSG bakes width/color into the line geometry.
    auto vsgNode = inet::vsg::createLine(nodePosition, nextHopPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED,
            lineColor, lineStyle, lineWidth);
    return new MulticastRouteVsgVisualization(vsgNode, route, node->getId(), nextHop->getId());
}

void RoutingTableVsgVisualizer::addMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::addMulticastRouteVisualization(routeVisualization);
    auto routeVsgVisualization = static_cast<const MulticastRouteVsgVisualization *>(routeVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(routeVsgVisualization->node);
}

void RoutingTableVsgVisualizer::removeMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization)
{
    RoutingTableVisualizerBase::removeMulticastRouteVisualization(routeVisualization);
    auto routeVsgVisualization = static_cast<const MulticastRouteVsgVisualization *>(routeVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(routeVsgVisualization->node)), ch.end());
}

void RoutingTableVsgVisualizer::refreshMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization) const
{
    // TODO: rebuild line label text when displayLabels is true, e.g.:
    //   auto text = getMulticastRouteVisualizationText(routeVisualization->route);
    // Same limitation as refreshRouteVisualization above.
}

} // namespace visualizer

} // namespace inet
