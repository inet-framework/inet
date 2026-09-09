//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSGGEOGRAPHICCOORDINATESYSTEM_H
#define __INET_OSGGEOGRAPHICCOORDINATESYSTEM_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

// Kept out of GeographicCoordinateSystem.h so that the common header stays free of
// osgEarth. osgEarth and Rocky each vendor their own copy of weejobs.h, which declares
// a global namespace alias, so the two cannot coexist in one translation unit: a Rocky
// source that included the common header would drag osgEarth in and fail to compile.
#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)

#include <osgEarth/MapNode>

namespace inet {

class INET_API OsgGeographicCoordinateSystem : public SimpleModule, public IGeographicCoordinateSystem
{
  protected:
    GeoCoord scenePosition = GeoCoord::NIL;
    Quaternion sceneOrientation = Quaternion::NIL;
    osgEarth::MapNode *mapNode = nullptr;
    osg::Matrixd locatorMatrix;
    osg::Matrixd inverseLocatorMatrix;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual GeoCoord getScenePosition() const override { return scenePosition; }
    virtual Quaternion getSceneOrientation() const override { return sceneOrientation; }

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const override;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const override;
};

} // namespace inet

#endif // WITH_OSGEARTH

#endif
