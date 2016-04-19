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

#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/networklayer/RouteCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(RouteCanvasVisualizer);

RouteCanvasVisualizer::CanvasRoute::CanvasRoute(const std::vector<int>& path, cPolylineFigure *figure) :
    Route(path),
    figure(figure)
{
}

RouteCanvasVisualizer::CanvasRoute::~CanvasRoute()
{
    delete figure;
}

void RouteCanvasVisualizer::initialize(int stage)
{
    RouteVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        canvasProjection = CanvasProjection::getCanvasProjection(visualizerTargetModule->getCanvas());
}

void RouteCanvasVisualizer::addRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    RouteVisualizerBase::addRoute(sourceAndDestination, route);
    auto canvasRoute = static_cast<const CanvasRoute *>(route);
    visualizerTargetModule->getCanvas()->addFigure(canvasRoute->figure);
}

void RouteCanvasVisualizer::removeRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    RouteVisualizerBase::removeRoute(sourceAndDestination, route);
    auto canvasRoute = static_cast<const CanvasRoute *>(route);
    visualizerTargetModule->getCanvas()->removeFigure(canvasRoute->figure);
}

const RouteVisualizerBase::Route *RouteCanvasVisualizer::createRoute(const std::vector<int>& path) const
{
    auto figure = new cPolylineFigure();
    figure->setLineWidth(lineWidth);
    figure->setEndArrowhead(cFigure::ARROW_BARBED);
    std::vector<cFigure::Point> points;
    for (auto id : path) {
        auto node = getSimulation()->getModule(id);
        points.push_back(canvasProjection->computeCanvasPoint(getPosition(node)));
    }
    figure->setPoints(points);
    auto color = cFigure::GOOD_DARK_COLORS[routes.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    figure->setLineColor(color);
    return new CanvasRoute(path, figure);
}

void RouteCanvasVisualizer::setAlpha(const Route *route, double alpha) const
{
    auto canvasRoute = static_cast<const CanvasRoute *>(route);
    auto figure = canvasRoute->figure;
    figure->setLineOpacity(alpha);
}

void RouteCanvasVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : routes) {
        auto route = static_cast<const CanvasRoute *>(it.second);
        auto figure = route->figure;
        for (int i = 0; i < route->path.size(); i++)
            if (node->getId() == route->path[i])
                figure->setPoint(i, canvasProjection->computeCanvasPoint(position + Coord(0, 0, route->offset)));
    }
}

} // namespace visualizer

} // namespace inet

