//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PRISM_H
#define __INET_PRISM_H

#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/object/Polygon.h"

namespace inet {

/**
 * This class represents 3 dimensional prism with a polygon base face.
 */
class INET_API Prism : public ShapeBase
{
  public:
    typedef std::vector<Polygon> Faces;
    typedef std::vector<Coord> Points;

  protected:
    double height;
    Polygon base;
    std::vector<Polygon> faces;
    std::vector<Coord> normalVectorsForFaces;

  protected:
    void genereateFaces();
    Coord computeOutwardNormalVector(unsigned int faceId) const;
    void computeOutwardNormalVectors();
    bool isVisibleFromPoint(unsigned int faceId, const Coord& point, const RotationMatrix& rotation) const;
    bool isVisibleFromView(unsigned int faceId, const RotationMatrix& viewRotation, const RotationMatrix& rotation) const;

  public:
    Prism() : height(0) {}
    Prism(double height, const Polygon& base);

    double getHeight() const { return height; }
    void setHeight(double height);

    const Polygon& getBase() const { return base; }
    void setBase(const Polygon& base);

    const Faces& getFaces() const { return faces; }

    virtual Coord computeBoundingBoxSize() const override;
    virtual bool computeIntersection(const LineSegment& lineSegment, Coord& intersection1, Coord& intersection2, Coord& normal1, Coord& normal2) const override;
    void computeVisibleFaces(std::vector<std::vector<Coord>>& faces, const RotationMatrix& rotation, const RotationMatrix& viewRotation) const;
};

} // namespace inet

#endif

