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
    Coord rotatedPoint = rotation.rotateVector(point);
    depth = rotatedPoint.z;
    return cFigure::Point(rotatedPoint.x * scale.x + translation.x, rotatedPoint.y * scale.y + translation.y);
}

Coord CanvasProjection::computeCanvasPointInverse(const cFigure::Point& point, double depth) const
{
    Coord p((point.x - translation.x) / scale.x, (point.y - translation.y) / scale.y, depth);
    return rotation.rotateVectorInverse(p);
}

CanvasProjection *CanvasProjection::getCanvasProjection(const cCanvas *canvas)
{
    static int canvasProjectionsHandle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::canvasProjections");
    auto& canvasProjections = getSimulationOrSharedDataManager()->getSharedVariable<std::map<const cCanvas *, CanvasProjection>>(canvasProjectionsHandle);
    return &canvasProjections[canvas];  // inserts element if not yet exists
}



} // namespace inet

