//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CANVASPROJECTION_H
#define __INET_CANVASPROJECTION_H

#include "inet/common/geometry/common/RotationMatrix.h"

namespace inet {

class INET_API CanvasProjection
{
  protected:
    RotationMatrix rotation;
    cFigure::Point scale;
    cFigure::Point translation;

  public:
    CanvasProjection() : scale(cFigure::Point(1, 1)) {}
    CanvasProjection(RotationMatrix rotation, cFigure::Point translation);
    virtual ~CanvasProjection();

    const RotationMatrix& getRotation() const { return rotation; }
    void setRotation(const RotationMatrix& rotation) { this->rotation = rotation; }

    const cFigure::Point& getScale() const { return scale; }
    void setScale(const cFigure::Point& scale) { this->scale = scale; }

    const cFigure::Point& getTranslation() const { return translation; }
    void setTranslation(const cFigure::Point& translation) { this->translation = translation; }

    cFigure::Point computeCanvasPoint(const Coord& point) const;
    cFigure::Point computeCanvasPoint(const Coord& point, double& depth) const;
    Coord computeCanvasPointInverse(const cFigure::Point& point, double depth) const;

    static CanvasProjection *getCanvasProjection(const cCanvas *canvas);
};

} // namespace inet

#endif

