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

#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PathCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

PathCanvasVisualizerBase::PathCanvasVisualization::PathCanvasVisualization(const std::vector<int>& path, cPolylineFigure *figure) :
    PathVisualization(path),
    figure(figure)
{
}

PathCanvasVisualizerBase::PathCanvasVisualization::~PathCanvasVisualization()
{
    delete figure;
}

void PathCanvasVisualizerBase::initialize(int stage)
{
    PathVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizerTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        pathGroup = new cGroupFigure("paths");
        pathGroup->setZIndex(zIndex);
        canvas->addFigure(pathGroup);
    }
}

const PathVisualizerBase::PathVisualization *PathCanvasVisualizerBase::createPathVisualization(const std::vector<int>& path) const
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
    auto color = cFigure::GOOD_DARK_COLORS[pathVisualizations.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    figure->setLineColor(color);
    return new PathCanvasVisualization(path, figure);
}

void PathCanvasVisualizerBase::addPathVisualization(std::pair<int, int> sourceAndDestination, const PathVisualization *pathVisualization)
{
    PathVisualizerBase::addPathVisualization(sourceAndDestination, pathVisualization);
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(pathVisualization);
    pathGroup->addFigure(pathCanvasVisualization->figure);
}

void PathCanvasVisualizerBase::removePathVisualization(std::pair<int, int> sourceAndDestination, const PathVisualization *pathVisualization)
{
    PathVisualizerBase::removePathVisualization(sourceAndDestination, pathVisualization);
    auto canvasPath = static_cast<const PathCanvasVisualization *>(pathVisualization);
    pathGroup->removeFigure(canvasPath->figure);
}

void PathCanvasVisualizerBase::setAlpha(const PathVisualization *path, double alpha) const
{
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(path);
    pathCanvasVisualization->figure->setLineOpacity(alpha);
}

void PathCanvasVisualizerBase::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : pathVisualizations) {
        auto pathVisualization = static_cast<const PathCanvasVisualization *>(it.second);
        auto figure = pathVisualization->figure;
        for (int i = 0; i < pathVisualization->moduleIds.size(); i++)
            if (node->getId() == pathVisualization->moduleIds[i])
                figure->setPoint(i, canvasProjection->computeCanvasPoint(position + Coord(0, 0, pathVisualization->offset)));
    }
}

} // namespace visualizer

} // namespace inet

