//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/CanvasProjection.h"

namespace inet {

CanvasProjection::CanvasProjection(RotationMatrix rotation, cFigure::Point translation) :
    rotation(rotation),
    scale(cFigure::Point(1, 1)),
    translation(translation)
{
}

CanvasProjection::~CanvasProjection()
{
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point) const
{
    double depth;
    return computeCanvasPoint(point, depth);
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point, double& depth) const
{
    // optional first projection stage (e.g. equirectangular), then the affine view transform
    Coord projected = mapProjection ? mapProjection->applyForward(point) : point;
    Coord rotatedPoint = rotation.rotateVector(projected);
    depth = rotatedPoint.z;
    return cFigure::Point(rotatedPoint.x * scale.x + translation.x, rotatedPoint.y * scale.y + translation.y);
}

Coord CanvasProjection::computeCanvasPointInverse(const cFigure::Point& point, double depth) const
{
    Coord p((point.x - translation.x) / scale.x, (point.y - translation.y) / scale.y, depth);
    Coord unrotated = rotation.rotateVectorInverse(p);
    return mapProjection ? mapProjection->applyInverse(unrotated) : unrotated;
}

cFigure::Point CanvasProjection::computeCanvasDirection(const Coord& point, const Coord& direction, DirectionProjection directionProjection) const
{
    double depth;
    return computeCanvasDirection(point, direction, depth, directionProjection);
}

cFigure::Point CanvasProjection::computeCanvasDirection(const Coord& point, const Coord& direction, double& depth, DirectionProjection directionProjection) const
{
    if (mapProjection)
        return mapProjection->computeCanvasDirection(point, direction, depth, directionProjection, rotation, scale);
    // no first projection stage: the linear part of the affine transform (rotation + scale), translation drops out
    Coord rotated = rotation.rotateVector(direction);
    depth = rotated.z;
    return cFigure::Point(rotated.x * scale.x, rotated.y * scale.y);
}

// Clips a polygon against one half-plane edge of a rectangle (Sutherland-Hodgman).
// edge: 0 = x>=bound (left), 1 = x<=bound (right), 2 = y>=bound (top), 3 = y<=bound (bottom).
static std::vector<cFigure::Point> clipAgainstEdge(const std::vector<cFigure::Point>& in, int edge, double bound)
{
    std::vector<cFigure::Point> out;
    int n = (int)in.size();
    if (n == 0)
        return out;
    auto inside = [&](const cFigure::Point& p) -> bool {
        switch (edge) {
            case 0: return p.x >= bound;
            case 1: return p.x <= bound;
            case 2: return p.y >= bound;
            default: return p.y <= bound;
        }
    };
    auto intersect = [&](const cFigure::Point& a, const cFigure::Point& b) -> cFigure::Point {
        double t = (edge <= 1) ? (bound - a.x) / (b.x - a.x) : (bound - a.y) / (b.y - a.y);
        return cFigure::Point(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
    };
    for (int i = 0; i < n; i++) {
        const cFigure::Point& cur = in[i];
        const cFigure::Point& prev = in[(i + n - 1) % n];
        bool curIn = inside(cur), prevIn = inside(prev);
        if (curIn) {
            if (!prevIn)
                out.push_back(intersect(prev, cur));
            out.push_back(cur);
        }
        else if (prevIn)
            out.push_back(intersect(prev, cur));
    }
    return out;
}

std::vector<cFigure::Point> CanvasProjection::clipPolygonToRect(std::vector<cFigure::Point> polygon, double xmin, double ymin, double xmax, double ymax)
{
    polygon = clipAgainstEdge(polygon, 0, xmin);
    polygon = clipAgainstEdge(polygon, 1, xmax);
    polygon = clipAgainstEdge(polygon, 2, ymin);
    polygon = clipAgainstEdge(polygon, 3, ymax);
    return polygon;
}

bool CanvasProjection::clipSegmentToRect(cFigure::Point& p0, cFigure::Point& p1, double xmin, double ymin, double xmax, double ymax)
{
    // Liang-Barsky line clipping.
    double dx = p1.x - p0.x, dy = p1.y - p0.y;
    double t0 = 0, t1 = 1;
    double p[4] = { -dx, dx, -dy, dy };
    double q[4] = { p0.x - xmin, xmax - p0.x, p0.y - ymin, ymax - p0.y };
    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            if (q[i] < 0)
                return false; // parallel to this edge and outside it
        }
        else {
            double t = q[i] / p[i];
            if (p[i] < 0) {
                if (t > t1) return false;
                if (t > t0) t0 = t;
            }
            else {
                if (t < t0) return false;
                if (t < t1) t1 = t;
            }
        }
    }
    cFigure::Point a(p0.x + t0 * dx, p0.y + t0 * dy);
    cFigure::Point b(p0.x + t1 * dx, p0.y + t1 * dy);
    p0 = a;
    p1 = b;
    return true;
}

bool CanvasProjection::isPointInsideClip(const cFigure::Point& point) const
{
    if (!clipEnabled)
        return true;
    return point.x >= clipRect.x && point.x <= clipRect.x + clipRect.width &&
           point.y >= clipRect.y && point.y <= clipRect.y + clipRect.height;
}

bool CanvasProjection::isRectVisibleInClip(const cFigure::Rectangle& bounds) const
{
    if (!clipEnabled)
        return true;
    // the figure bounding box overlaps the clip rect (i.e. any part is inside)
    return bounds.x <= clipRect.x + clipRect.width && bounds.x + bounds.width >= clipRect.x &&
           bounds.y <= clipRect.y + clipRect.height && bounds.y + bounds.height >= clipRect.y;
}

bool CanvasProjection::clipLine(cFigure::Point& p0, cFigure::Point& p1) const
{
    if (!clipEnabled)
        return true;
    return clipSegmentToRect(p0, p1, clipRect.x, clipRect.y, clipRect.x + clipRect.width, clipRect.y + clipRect.height);
}

std::vector<cFigure::Point> CanvasProjection::clipPolygon(const std::vector<cFigure::Point>& polygon) const
{
    if (!clipEnabled)
        return polygon;
    return clipPolygonToRect(polygon, clipRect.x, clipRect.y, clipRect.x + clipRect.width, clipRect.y + clipRect.height);
}

CanvasProjection *CanvasProjection::getCanvasProjection(const cCanvas *canvas)
{
    static int canvasProjectionsHandle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::canvasProjections");
    auto& canvasProjections = getSimulationOrSharedDataManager()->getSharedVariable<std::map<const cCanvas *, CanvasProjection>>(canvasProjectionsHandle);
    return &canvasProjections[canvas];  // inserts element if not yet exists
}



} // namespace inet

