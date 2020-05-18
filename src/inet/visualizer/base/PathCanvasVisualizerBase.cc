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
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PathCanvasVisualizerBase.h"

namespace inet {

namespace visualizer {

static inline double determinant(double a1, double a2, double b1, double b2)
{
    return a1 * b2 - a2 * b1;
}

static Coord intersectLines(const LineSegment& segment1, const LineSegment& segment2)
{
    const Coord& begin1 = segment1.getPoint1();
    const Coord& end1 = segment1.getPoint2();
    const Coord& begin2 = segment2.getPoint1();
    const Coord& end2 = segment2.getPoint2();
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

bool isPointOnSegment(const LineSegment& segment, const Coord& point)
{
    auto& p1 = segment.getPoint1();
    auto& p2 = segment.getPoint2();
    return (std::min(p1.x, p2.x) <= point.x && point.x <= std::max(p1.x, p2.x) &&
            std::min(p1.y, p2.y) <= point.y && point.y <= std::max(p1.y, p2.y));
}

PathCanvasVisualizerBase::PathCanvasVisualization::PathCanvasVisualization(const char *label, const std::vector<int>& path, LabeledPolylineFigure *figure) :
    PathVisualization(label, path),
    figure(figure)
{
}

PathCanvasVisualizerBase::PathCanvasVisualization::~PathCanvasVisualization()
{
    delete figure;
}

PathCanvasVisualizerBase::~PathCanvasVisualizerBase()
{
    if (displayRoutes)
        removeAllPathVisualizations();
}

void PathCanvasVisualizerBase::initialize(int stage)
{
    PathVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
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
        for (size_t index = 1; index < moduleIds.size(); index++) {
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
        for (size_t index = 0; index < segments.size(); index++) {
            if (index == 0)
                points.push_back(canvasProjection->computeCanvasPoint(segments[index].getPoint1()));
            if (index > 0) {
                auto& segment1 = segments[index - 1];
                auto& segment2 = segments[index];
                Coord intersection = intersectLines(segment1, segment2);
                if (std::isfinite(intersection.x) && std::isfinite(intersection.y)) {
                    if (isPointOnSegment(segment1, intersection) && isPointOnSegment(segment2, intersection))
                        points.push_back(canvasProjection->computeCanvasPoint(intersection));
                    else {
                        double distance = segment1.getPoint2().distance(segment2.getPoint1());
                        double distance1 = intersection.distance(segment1.getPoint2());
                        double distance2 = intersection.distance(segment2.getPoint1());
                        if (distance1 + distance2 < 4 * distance)
                            points.push_back(canvasProjection->computeCanvasPoint(intersection));
                        else {
                            points.push_back(canvasProjection->computeCanvasPoint(segment1.getPoint2()));
                            points.push_back(canvasProjection->computeCanvasPoint(segment2.getPoint1()));
                        }
                    }
                }
                else {
                    points.push_back(canvasProjection->computeCanvasPoint(segment1.getPoint2()));
                    points.push_back(canvasProjection->computeCanvasPoint(segment2.getPoint1()));
                }
            }
            if (index == segments.size() - 1)
                points.push_back(canvasProjection->computeCanvasPoint(segments[index].getPoint2()));
        }
        pathCanvasVisualization->figure->setPoints(points);
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(pathVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PathVisualizerBase::PathVisualization *PathCanvasVisualizerBase::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    auto figure = new LabeledPolylineFigure("path");
    auto polylineFigure = figure->getPolylineFigure();
    polylineFigure->setSmooth(lineSmooth);
    polylineFigure->setLineWidth(lineWidth);
    polylineFigure->setLineStyle(lineStyle);
    polylineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    auto lineColor = lineColorSet.getColor(pathVisualizations.size());
    polylineFigure->setLineColor(lineColor);
    auto labelFigure = figure->getLabelFigure();
    labelFigure->setFont(labelFont);
    labelFigure->setColor(isEmpty(labelColorAsString) ? lineColor : labelColor);
    return new PathCanvasVisualization(label, path, figure);
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
    pathCanvasVisualization->figure->getPolylineFigure()->setLineOpacity(alpha);
}

void PathCanvasVisualizerBase::refreshPathVisualization(const PathVisualization *pathVisualization, cPacket *packet)
{
    PathVisualizerBase::refreshPathVisualization(pathVisualization, packet);
    auto pathCanvasVisualization = static_cast<const PathCanvasVisualization *>(pathVisualization);
    auto text = getPathVisualizationText(pathVisualization, packet);
    pathCanvasVisualization->figure->getLabelFigure()->setText(text.c_str());
}

} // namespace visualizer

} // namespace inet

