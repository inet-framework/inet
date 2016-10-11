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

#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/networklayer/RoutingTableOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(RoutingTableOsgVisualizer);

RoutingTableOsgVisualizer::RouteOsgVisualization::RouteOsgVisualization(osg::Node *node, int nodeModuleId, int nextHopModuleId) :
    RouteVisualization(nodeModuleId, nextHopModuleId),
    node(node)
{
}

RoutingTableOsgVisualizer::RouteOsgVisualization::~RouteOsgVisualization()
{
    // TODO: delete node;
}

void RoutingTableOsgVisualizer::addRouteVisualization(std::pair<int, int> nodeAndNextHop, const RouteVisualization *route)
{
    RoutingTableVisualizerBase::addRouteVisualization(nodeAndNextHop, route);
    auto osgRoute = static_cast<const RouteOsgVisualization *>(route);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(osgRoute->node);
}

void RoutingTableOsgVisualizer::removeRouteVisualization(const RouteVisualization *route)
{
    RoutingTableVisualizerBase::removeRouteVisualization(route);
    auto osgRoute = static_cast<const RouteOsgVisualization *>(route);
    auto node = osgRoute->node;
    node->getParent(0)->removeChild(node);
}

void RoutingTableOsgVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : routeVisualizations) {
        auto route = static_cast<const RouteOsgVisualization *>(it.second);
        auto group = static_cast<osg::Group *>(route->node);
        auto geode = static_cast<osg::Geode *>(group->getChild(0));
        auto geometry = static_cast<osg::Geometry *>(geode->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(geometry->getVertexArray());
        if (node->getId() == it.first.first)
            vertices->at(0) = osg::Vec3d(position.x, position.y, position.z);
        else if (node->getId() == it.first.second) {
            osg::Vec3d p(position.x, position.y, position.z);
            vertices->at(1) = p;
            auto autoTransform = static_cast<osg::AutoTransform *>(group->getChild(1));
            if (autoTransform != nullptr)
                autoTransform->setPosition(p);
        }
        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableOsgVisualizer::createRouteVisualization(cModule *node, cModule *nextHop) const
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
    return new RouteOsgVisualization(osgNode, node->getId(), nextHop->getId());
}

} // namespace visualizer

} // namespace inet

