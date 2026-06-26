//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMAPPROJECTION_H
#define __INET_IMAPPROJECTION_H

#include "inet/common/geometry/common/RotationMatrix.h"

namespace inet {

/**
 * Abstract first projection stage for CanvasProjection: an optional, possibly non-linear map from
 * the scene coordinate to the pre-affine coordinate that the canvas view transform (rotation, scale,
 * translation) is then applied to. When a CanvasProjection has no map projection the first stage is
 * the identity (the historical no-projection behavior). The concrete implementation
 * (EquirectangularProjection) owns all geographic/geodetic knowledge; CanvasProjection only delegates
 * through this interface and never references the projection specifics directly.
 */
class INET_API IMapProjection
{
  public:
    // Selects how computeCanvasDirection() projects a direction (tangent) vector on a distorting map
    // projection, decoupling two properties: conformal (preserve true angles, isotropic, no shear) and
    // isometric (on-screen length uses one global metric scale, no blow-up toward the poles).
    //  - DIRECTION_PROJECTED:   non-conformal + non-isometric: the full map Jacobian (e.g. longitude
    //                           stretches by 1/cos(latitude) toward the poles);
    //  - DIRECTION_TRUE_LENGTH: non-conformal + isometric: the map heading (sheared direction) at true metric length;
    //  - DIRECTION_ISOMETRIC:   conformal + isometric (a single uniform local scale): true heading and length.
    enum DirectionProjection { DIRECTION_PROJECTED, DIRECTION_TRUE_LENGTH, DIRECTION_ISOMETRIC };

    virtual ~IMapProjection() {}

    // The first projection stage and its inverse, applied to a scene coordinate before/after the
    // canvas view transform.
    virtual Coord applyForward(const Coord& scenePoint) const = 0;
    virtual Coord applyInverse(const Coord& projectedPoint) const = 0;

    // Projects a direction (tangent) vector anchored at the given scene point to a canvas-space vector,
    // according to the given mode. The canvas view affine (rotation and scale; translation drops out for
    // a direction) is supplied by the caller (CanvasProjection), so the affine stays owned there. The
    // depth output is the out-of-plane (toward/away the viewer) component, positive toward the viewer.
    virtual cFigure::Point computeCanvasDirection(const Coord& point, const Coord& direction, double& depth,
            DirectionProjection directionProjection, const RotationMatrix& rotation, const cFigure::Point& scale) const = 0;
};

} // namespace inet

#endif
