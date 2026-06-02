//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOCENTRICCOORDINATESYSTEM_H
#define __INET_GEOCENTRICCOORDINATESYSTEM_H

#include "inet/common/SimpleModule.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

namespace inet {

/**
 * A build-independent geographic coordinate system in which the scene coordinate
 * frame coincides with the Earth-Centered Earth-Fixed (ECEF) frame: the origin is
 * at the center of the Earth, the X axis points to the prime meridian on the
 * equator, the Z axis points to the geographic north pole, and units are meters.
 *
 * Unlike SimpleGeographicCoordinateSystem (a local tangent plane) this is globally
 * correct, so slant ranges and elevation angles are valid across the whole globe,
 * which is required for satellite scenarios. Unlike OsgGeographicCoordinateSystem
 * it does not depend on osgEarth, so it is available in any build.
 *
 * Conversions use the WGS84 ellipsoid (see Wgs84.h).
 */
class INET_API GeocentricCoordinateSystem : public SimpleModule, public IGeographicCoordinateSystem
{
  public:
    virtual GeoCoord getScenePosition() const override;
    virtual Quaternion getSceneOrientation() const override { return Quaternion::IDENTITY; }

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const override;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const override;
};

} // namespace inet

#endif

