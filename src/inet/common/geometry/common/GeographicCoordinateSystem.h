//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOGRAPHICCOORDINATESYSTEM_H
#define __INET_GEOGRAPHICCOORDINATESYSTEM_H

#include "inet/common/SimpleModule.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"

#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)
#include <osgEarth/MapNode>
#endif

namespace inet {

class INET_API GeoCoord
{
  public:
    static const GeoCoord NIL;

  public:
    /** @name latitude, longitude and altitude coordinate of the position. */
    /*@{*/
    deg latitude;
    deg longitude;
    m altitude;
    /*@}*/

  public:
    GeoCoord(deg latitude, deg longitude, m altitude) : latitude(latitude), longitude(longitude), altitude(altitude) {}
};

class INET_API IGeographicCoordinateSystem
{
  public:
    virtual GeoCoord getScenePosition() const = 0;
    virtual Quaternion getSceneOrientation() const = 0;

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const = 0;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const = 0;
};

/**
 * A simple geographic coordinate system that uses a flat Earth approximation
 * with a local tangent plane at a specified anchor point.
 *
 * WARNING: This implementation is globally NOT accurate as it assumes Earth is a perfect
 * sphere and uses a simple flat-plane projection. It does not account for Earth's
 * ellipsoidal shape or local terrain variations. Usage is DISCOURAGED for applications
 * requiring global geographic accuracy, but it is fine to be used on city-sized areas.
 *
 * Consider using Wgs84EcefGeographicCoordinateSystem for more accurate results.
 */

class INET_API SimpleGeographicCoordinateSystem : public SimpleModule, public IGeographicCoordinateSystem
{
  protected:
    double metersPerDegree = 111320;
    deg sceneLatitude = deg(NaN);
    deg sceneLongitude = deg(NaN);
    m sceneAltitude = m(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual GeoCoord getScenePosition() const override { return GeoCoord(sceneLatitude, sceneLongitude, sceneAltitude); }
    virtual Quaternion getSceneOrientation() const override { return Quaternion::IDENTITY; }

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const override;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const override;
};

/**
 * A geographic coordinate system in which the scene coordinate frame
 * coincides with the Earth-Centered Earth-Fixed (ECEF) frame: the origin is
 * at the center of the Earth, the X axis points to the prime meridian on the
 * equator, the Z axis points to the geographic north pole, and units are meters.
 *
 * Unlike SimpleGeographicCoordinateSystem (a local tangent plane) this is globally
 * correct, so slant ranges and elevation angles are valid across the whole globe,
 * which is required for satellite scenarios. Unlike OsgGeographicCoordinateSystem
 * it does not depend on osgEarth, so it is available in any build.
 *
 * Conversions use the WGS84 ellipsoid, for accuracy limitations, see Wgs84.h 
 */
class INET_API Wgs84EcefGeographicCoordinateSystem : public SimpleModule, public IGeographicCoordinateSystem
{
  public:
    virtual GeoCoord getScenePosition() const override;
    virtual Quaternion getSceneOrientation() const override { return Quaternion::IDENTITY; }

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const override;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const override;
};

/**
 * An accurate LOCAL geographic coordinate system: the scene coordinate frame is a
 * local tangent plane pinned to a geographic anchor point
 * (sceneLatitude/Longitude/Altitude) and optionally rotated
 * (sceneHeading/Elevation/Bank). Scene coordinates are in meters relative to that
 * anchor.
 *
 * This provides the same capability as OsgGeographicCoordinateSystem (an anchored,
 * orientable local tangent plane with the same parameters), but computes the
 * conversions directly from the WGS84 ellipsoid instead of relying on osgEarth, so
 * it is available in any build. The scene frame follows INET's convention: by
 * default (sceneHeading = 90deg) the X axis points east, the Y axis points south and
 * the Z axis points up at the anchor. (OsgGeographicCoordinateSystem instead yields a
 * right-handed East-North-Up frame, which renders correctly in the 3D osgEarth view;
 * this one targets the flat 2D scene, where +Y is drawn downward, so north maps to -Y.)
 *
 * Unlike Wgs84EcefGeographicCoordinateSystem (whose scene frame is global ECEF),
 * scene coordinates here stay small and centered on the anchor, which is convenient
 * for local scenes; conversions remain globally correct because they go through ECEF
 * and the WGS84 ellipsoid. For accuracy limitations, see Wgs84.h
 */
class INET_API Wgs84AnchoredGeographicCoordinateSystem : public SimpleModule, public IGeographicCoordinateSystem
{
  protected:
    GeoCoord scenePosition = GeoCoord::NIL;
    Quaternion sceneOrientation = Quaternion::IDENTITY;

    /** Precomputed anchor position in ECEF and the local <-> ECEF rotations. */
    Coord originEcef = Coord::ZERO;
    Quaternion localToEcefRotation = Quaternion::IDENTITY;
    Quaternion ecefToLocalRotation = Quaternion::IDENTITY;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual GeoCoord getScenePosition() const override { return scenePosition; }
    virtual Quaternion getSceneOrientation() const override { return sceneOrientation; }

    virtual Coord computeSceneCoordinate(const GeoCoord& geographicCoordinate) const override;
    virtual GeoCoord computeGeographicCoordinate(const Coord& sceneCoordinate) const override;
};

#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)

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

#endif // WITH_OSGEARTH

} // namespace inet

#endif

