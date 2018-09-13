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

#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/networklayer/RoutingTableOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(RoutingTableOsgVisualizer);

#ifdef WITH_OSG

RoutingTableOsgVisualizer::RouteOsgVisualization::RouteOsgVisualization(osg::Node *node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
    RouteVisualization(route, nodeModuleId, nextHopModuleId),
    node(node)
{
}

RoutingTableOsgVisualizer::RouteOsgVisualization::~RouteOsgVisualization()
{
    // TODO: delete node;
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
    // TODO:
    // auto routeOsgVisualization = static_cast<const RouteOsgVisualization *>(routeVisualization);
    // auto text = getRouteVisualizationText(routeVisualization->route);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

