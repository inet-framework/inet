//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SATELLITEMOBILITY_H
#define __INET_SATELLITEMOBILITY_H

#include "inet/common/INETDefs.h"


#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/mobility/geographic/common/TleFile.h"

namespace inet {

/**
 * Mobility model that propagates a satellite orbit from a TLE (two-line element)
 * set using the embedded SGP4 implementation, and reports the satellite position
 * in the simulation scene through a geographic coordinate system.
 *
 * The model reads its own TLE file and selects a satellite by name, NORAD catalog
 * number, or index, so it can be used standalone in a NED file. It can also be
 * created and configured by a ~SatelliteController, which distributes the common
 * UTC epoch to all satellites.
 *
 * Pipeline at each update: SGP4 propagation -> TEME (ECI) position (km) -> ECEF via
 * the GMST rotation at the current UTC -> WGS84 geodetic -> scene coordinate via
 * the geographic coordinate system. Velocity is obtained by finite-differencing the
 * scene position. See Wgs84.h for the (GMST-only) Earth-orientation conventions.
 */
class INET_API SatelliteMobility : public MovingMobilityBase
{
  public:
    enum Pointing { POINTING_NONE, POINTING_NADIR, POINTING_VELOCITY };

  protected:
    /** Geographic coordinate system that maps geodetic coordinates to the scene. */
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;

    /** The selected satellite's SGP4 element record. */
    TleFile tleFile;
    elsetrec satrec;

    /** UTC Julian date corresponding to simulation time 0. */
    double epochJulianDate = 0;

    Pointing pointing = POINTING_NADIR;
    double maxSpeed = 8000; // m/s, conservative upper bound for low Earth orbit

    /** Cached geodetic (sub-satellite) position, exposed for map/sky visualizers. */
    GeoCoord lastGeoPosition = GeoCoord::NIL;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;
    virtual void orient() override;

    /** Propagates the orbit and returns the scene position at the given UTC Julian date. */
    virtual Coord computeScenePosition(double julianDate, GeoCoord& geographicPosition);

  public:
    virtual double getMaxSpeed() const override { return maxSpeed; }

    /** Returns the cached geodetic (latitude/longitude/altitude) sub-satellite position. */
    virtual const GeoCoord& getGeographicPosition() const { return lastGeoPosition; }
};

} // namespace inet


#endif

