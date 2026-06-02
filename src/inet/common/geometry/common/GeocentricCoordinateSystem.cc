//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/GeocentricCoordinateSystem.h"

#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

Define_Module(GeocentricCoordinateSystem);

GeoCoord GeocentricCoordinateSystem::getScenePosition() const
{
    // The scene origin is the center of the Earth, which has no meaningful
    // geodetic position; report the sub-origin point on the prime meridian.
    return GeoCoord(deg(0), deg(0), m(-wgs84::semiMajorAxis));
}

Coord GeocentricCoordinateSystem::computeSceneCoordinate(const GeoCoord& geographicCoordinate) const
{
    return wgs84::geodeticToEcef(geographicCoordinate);
}

GeoCoord GeocentricCoordinateSystem::computeGeographicCoordinate(const Coord& sceneCoordinate) const
{
    return wgs84::ecefToGeodetic(sceneCoordinate);
}

} // namespace inet

