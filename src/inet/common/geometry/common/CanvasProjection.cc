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

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point, bool applyMapProjection) const
{
    double depth;
    return computeCanvasPoint(point, depth, applyMapProjection);
}

cFigure::Point CanvasProjection::computeCanvasPoint(const Coord& point, double& depth, bool applyMapProjection) const
{
    // optional first projection stage (e.g. equirectangular), then the affine view transform; pass
    // applyMapProjection = false to skip the first stage (see the header)
    Coord projected = (applyMapProjection && mapProjection) ? mapProjection->applyForward(point) : point;
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

CanvasProjection *CanvasProjection::getCanvasProjection(const cCanvas *canvas)
{
    static int canvasProjectionsHandle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::canvasProjections");
    auto& canvasProjections = getSimulationOrSharedDataManager()->getSharedVariable<std::map<const cCanvas *, CanvasProjection>>(canvasProjectionsHandle);
    return &canvasProjections[canvas];  // inserts element if not yet exists
}



} // namespace inet

