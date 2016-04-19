//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGUtils.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/networklayer/RouteOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(RouteOsgVisualizer);

#ifdef WITH_OSG

RouteOsgVisualizer::OsgRoute::OsgRoute(const std::vector<int>& path, osg::Node *node) :
    Route(path),
    node(node)
{
}

RouteOsgVisualizer::OsgRoute::~OsgRoute()
{
    // TODO: delete node;
}

void RouteOsgVisualizer::addRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    RouteVisualizerBase::addRoute(sourceAndDestination, route);
    auto osgRoute = static_cast<const OsgRoute *>(route);
    auto scene = inet::osg::getScene(visualizerTargetModule);
    scene->addChild(osgRoute->node);
}

void RouteOsgVisualizer::removeRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    RouteVisualizerBase::removeRoute(sourceAndDestination, route);
    auto osgRoute = static_cast<const OsgRoute *>(route);
    auto node = osgRoute->node;
    node->getParent(0)->removeChild(node);
}

const RouteVisualizerBase::Route *RouteOsgVisualizer::createRoute(const std::vector<int>& path) const
{
    std::vector<Coord> points;
    for (auto id : path) {
        auto node = getSimulation()->getModule(id);
        points.push_back(getPosition(node));
    }
    auto node = inet::osg::createPolyline(points, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    auto color = cFigure::GOOD_DARK_COLORS[routes.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    auto stateSet = inet::osg::createStateSet(color, 1, false);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    node->setStateSet(stateSet);
    return new OsgRoute(path, node);
}

void RouteOsgVisualizer::setAlpha(const Route *route, double alpha) const
{
    auto osgRoute = static_cast<const OsgRoute *>(route);
    auto node = osgRoute->node;
    auto stateSet = node->getOrCreateStateSet();
    auto material = static_cast<osg::Material *>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void RouteOsgVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : routes) {
        auto route = static_cast<const OsgRoute *>(it.second);
        auto group = static_cast<osg::Group *>(route->node);
        auto geode = static_cast<osg::Geode *>(group->getChild(0));
        auto geometry = static_cast<osg::Geometry *>(geode->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(geometry->getVertexArray());
        auto autoTransform = dynamic_cast<osg::AutoTransform *>(group->getChild(1));
        for (int i = 0; i < route->path.size(); i++) {
            if (node->getId() == route->path[i]) {
                osg::Vec3d p(position.x, position.y, position.z + route->offset);
                vertices->at(i) = p;
                if (autoTransform != nullptr && i == route->path.size() - 1)
                    autoTransform->setPosition(p);
            }
        }
        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

