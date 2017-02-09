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

#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PathCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

static inline double determinant(double a1, double a2, double b1, double b2)
{
    return a1 * b2 - a2 * b1;
}

static Coord intersectLines(const Coord& begin1, const Coord& end1, const Coord& begin2, const Coord& end2)
{
    double x1 = begin1.x;
    double y1 = begin1.y;
    double x2 = end1.x;
    double y2 = end1.y;
    double x3 = begin2.x;
    double y3 = begin2.y;
    double x4 = end2.x;
    double y4 = end2.y;
    double a = determinant(x1, y1, x2, y2);
    double b = determinant(x3, y3, x4, y4);
    double c = determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double x = determinant(a, x1 - x2, b, x3 - x4) / c;
    double y = determinant(a, y1 - y2, b, y3 - y4) / c;
    return Coord(x, y, 0);
}

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

void PathCanvasVisualizerBase::refreshDisplay() const
{
    PathVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : pathVisualizations) {
        auto pathVisualization = it.second;
        auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(pathVisualization);
        auto moduleIds = pathCanvasVisualization->moduleIds;
        std::vector<LineSegment> segments;
        for (int index = 1; index < moduleIds.size(); index++) {
            auto fromModuleId = moduleIds[index - 1];
            auto toModuleId = moduleIds[index];
            auto fromModule = simulation->getModule(fromModuleId);
            auto toModule = simulation->getModule(toModuleId);
            auto fromPosition = getContactPosition(fromModule, getPosition(toModule), lineContactMode, lineContactSpacing);
            auto toPosition = getContactPosition(toModule, getPosition(fromModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(fromModuleId, toModuleId, fromPosition, toPosition, lineShiftMode, pathVisualization->shiftOffsets[index - 1]) * lineShift;
            segments.push_back(LineSegment(fromPosition + shift, toPosition + shift));
        }
        std::vector<cFigure::Point> points;
        for (int index = 0; index < segments.size(); index++) {
            if (index == 0)
                points.push_back(canvasProjection->computeCanvasPoint(segments[index].getPoint1()));
            if (index > 0) {
                Coord intersection = intersectLines(segments[index].getPoint1(), segments[index].getPoint2(), segments[index - 1].getPoint1(), segments[index - 1].getPoint2());
                points.push_back(canvasProjection->computeCanvasPoint(intersection));
            }
            if (index == segments.size() - 1)
                points.push_back(canvasProjection->computeCanvasPoint(segments[index].getPoint2()));
        }
        pathCanvasVisualization->figure->setPoints(points);
    }
    visualizerTargetModule->getCanvas()->setAnimationSpeed(pathVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PathVisualizerBase::PathVisualization *PathCanvasVisualizerBase::createPathVisualization(const std::vector<int>& path) const
{
    auto figure = new cPolylineFigure("path");
    figure->setLineWidth(lineWidth);
    figure->setLineStyle(lineStyle);
    figure->setEndArrowhead(cFigure::ARROW_BARBED);
    figure->setLineColor(lineColorSet.getColor(pathVisualizations.size()));
    return new PathCanvasVisualization(path, figure);
}

void PathCanvasVisualizerBase::addPathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::addPathVisualization(pathVisualization);
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(pathVisualization);
    lineManager->addModulePath(pathVisualization);
    pathGroup->addFigure(pathCanvasVisualization->figure);
}

void PathCanvasVisualizerBase::removePathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::removePathVisualization(pathVisualization);
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(pathVisualization);
    lineManager->removeModulePath(pathVisualization);
    pathGroup->removeFigure(pathCanvasVisualization->figure);
}

void PathCanvasVisualizerBase::setAlpha(const PathVisualization *path, double alpha) const
{
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(path);
    pathCanvasVisualization->figure->setLineOpacity(alpha);
}

} // namespace visualizer

} // namespace inet

