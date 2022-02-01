//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/base/TreeCanvasVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/mobility/contract/IMobility.h"

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

static bool isPointOnSegment(const LineSegment& segment, const Coord& point)
{
    auto& p1 = segment.getPoint1();
    auto& p2 = segment.getPoint2();
    return std::min(p1.x, p2.x) <= point.x && point.x <= std::max(p1.x, p2.x) &&
           std::min(p1.y, p2.y) <= point.y && point.y <= std::max(p1.y, p2.y);
}

TreeCanvasVisualizerBase::TreeCanvasVisualization::TreeCanvasVisualization(const std::vector<std::vector<int>>& tree, std::vector<LabeledPolylineFigure *>& figures) :
    TreeVisualization(tree),
    figures(figures)
{
}

TreeCanvasVisualizerBase::TreeCanvasVisualization::~TreeCanvasVisualization()
{
    for (auto figure : figures)
        delete figure;
}

void TreeCanvasVisualizerBase::initialize(int stage)
{
    TreeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        treeGroup = new cGroupFigure("trees");
        treeGroup->setZIndex(zIndex);
        canvas->addFigure(treeGroup);
    }
}

void TreeCanvasVisualizerBase::refreshDisplay() const
{
    TreeVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : treeVisualizations) {
        auto treeVisualization = it.second;
        auto treeCanvasVisualization = static_cast<const TreeCanvasVisualization *>(treeVisualization);
        for (int i = 0; i < treeCanvasVisualization->paths.size(); i++) {
            auto& path = treeCanvasVisualization->paths[i];
            auto figure = treeCanvasVisualization->figures[i];
            auto moduleIds = path.moduleIds;
            std::vector<LineSegment> segments;
            for (size_t index = 1; index < moduleIds.size(); index++) {
                auto fromModuleId = moduleIds[index - 1];
                auto toModuleId = moduleIds[index];
                auto fromModule = simulation->getModule(fromModuleId);
                auto toModule = simulation->getModule(toModuleId);
                auto fromPosition = getContactPosition(fromModule, getPosition(toModule), lineContactMode, lineContactSpacing);
                auto toPosition = getContactPosition(toModule, getPosition(fromModule), lineContactMode, lineContactSpacing);
                auto shift = lineManager->getLineShift(fromModuleId, toModuleId, fromPosition, toPosition, lineShiftMode, path.shiftOffsets[index - 1]) * lineShift;
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
            figure->setPoints(points);
        }
    }
}

const TreeVisualizerBase::TreeVisualization *TreeCanvasVisualizerBase::createTreeVisualization(const std::vector<std::vector<int>>& tree) const
{
    std::vector<LabeledPolylineFigure *> figures;
    for (int i = 0; i < tree.size(); i++) {
        auto figure = new LabeledPolylineFigure("tree");
        auto polylineFigure = figure->getPolylineFigure();
        polylineFigure->setSmooth(lineSmooth);
        polylineFigure->setLineWidth(lineWidth);
        polylineFigure->setLineStyle(lineStyle);
        polylineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
        auto lineColor = lineColorSet.getColor(treeVisualizations.size());
        polylineFigure->setLineColor(lineColor);
        figures.push_back(figure);
    }
    return new TreeCanvasVisualization(tree, figures);
}

void TreeCanvasVisualizerBase::addTreeVisualization(const TreeVisualization *treeVisualization)
{
    TreeVisualizerBase::addTreeVisualization(treeVisualization);
    auto treeCanvasVisualization = static_cast<const TreeCanvasVisualization *>(treeVisualization);
    for (auto& path : treeVisualization->paths)
        lineManager->addModulePath(&path);
    for (auto figure : treeCanvasVisualization->figures)
        treeGroup->addFigure(figure);
}

void TreeCanvasVisualizerBase::removeTreeVisualization(const TreeVisualization *treeVisualization)
{
    TreeVisualizerBase::removeTreeVisualization(treeVisualization);
    auto treeCanvasVisualization = static_cast<const TreeCanvasVisualization *>(treeVisualization);
    for (auto& path : treeVisualization->paths)
        lineManager->removeModulePath(&path);
    for (auto figure : treeCanvasVisualization->figures)
        treeGroup->removeFigure(figure);
}

} // namespace visualizer

} // namespace inet

