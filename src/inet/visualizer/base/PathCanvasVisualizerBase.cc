//
// Copyright (C) 2016 OpenSim Ltd.
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

PathCanvasVisualizerBase::CanvasPath::CanvasPath(const std::vector<int>& path, cPolylineFigure *figure) :
    Path(path),
    figure(figure)
{
}

PathCanvasVisualizerBase::CanvasPath::~CanvasPath()
{
    delete figure;
}

void PathCanvasVisualizerBase::initialize(int stage)
{
    PathVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        canvasProjection = CanvasProjection::getCanvasProjection(visualizerTargetModule->getCanvas());
}

void PathCanvasVisualizerBase::addPath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    PathVisualizerBase::addPath(sourceAndDestination, path);
    auto canvasPath = static_cast<const CanvasPath *>(path);
    visualizerTargetModule->getCanvas()->addFigure(canvasPath->figure);
}

void PathCanvasVisualizerBase::removePath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    PathVisualizerBase::removePath(sourceAndDestination, path);
    auto canvasPath = static_cast<const CanvasPath *>(path);
    visualizerTargetModule->getCanvas()->removeFigure(canvasPath->figure);
}

const PathVisualizerBase::Path *PathCanvasVisualizerBase::createPath(const std::vector<int>& path) const
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
    auto color = cFigure::GOOD_DARK_COLORS[paths.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    figure->setLineColor(color);
    return new CanvasPath(path, figure);
}

void PathCanvasVisualizerBase::setAlpha(const Path *path, double alpha) const
{
    auto canvasPath = static_cast<const CanvasPath *>(path);
    auto figure = canvasPath->figure;
    figure->setLineOpacity(alpha);
}

void PathCanvasVisualizerBase::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : paths) {
        auto path = static_cast<const CanvasPath *>(it.second);
        auto figure = path->figure;
        for (int i = 0; i < path->moduleIds.size(); i++)
            if (node->getId() == path->moduleIds[i])
                figure->setPoint(i, canvasProjection->computeCanvasPoint(position + Coord(0, 0, path->offset)));
    }
}

} // namespace visualizer

} // namespace inet

