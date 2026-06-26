//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SATELLITEMOBILITY_H
#define __INET_SATELLITEMOBILITY_H

#include "inet/common/INETDefs.h"


#include "3rdparty/sgp4/SGP4.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * Mobility model that propagates a satellite orbit from a TLE (two-line element)
 * set using the embedded SGP4 implementation, and reports the satellite position
 * in the simulation scene through a geographic coordinate system.
 *
 * The model takes the raw TLE data from the 'tleData' parameter (e.g.
 * tleData = readFile("...")) and selects a satellite by name, NORAD catalog number,
 * or index. Each satellite is configured individually: set its TLE data, the selected
 * satellite, and the UTC epoch mapped to simulation time 0 directly as parameters.
 *
 * Pipeline at each update: SGP4 propagation -> TEME (ECI) position (km) -> ECEF via
 * the GMST rotation at the current UTC -> WGS84 geodetic -> scene coordinate via
 * the geographic coordinate system. Velocity is obtained by finite-differencing the
 * scene position. See Wgs84.h for the (GMST-only) Earth-orientation conventions.
 */
class INET_API SatelliteMobility : public MovingMobilityBase
{
  public:
    enum AttitudeMode { ATTITUDE_EARTH_FIXED, ATTITUDE_STAR_FIXED, ATTITUDE_NADIR, ATTITUDE_ZENITH, ATTITUDE_VELOCITY };

  protected:
    /** Geographic coordinate system that maps geodetic coordinates to the scene. */
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;

    /** The selected satellite's SGP4 element record. */
    elsetrec satrec;

    /** UTC Julian date corresponding to simulation time 0. */
    double epochJulianDate = 0;

    AttitudeMode attitudeMode = ATTITUDE_NADIR;
    double maxSpeed = 8000; // m/s, conservative upper bound for low Earth orbit

    /** UTC Julian date of the latest move(), used by the star_fixed (inertial) attitude. */
    double currentJulianDate = 0;

    /** Cached geodetic (sub-satellite) position, exposed for map/sky visualizers. */
    GeoCoord lastGeoPosition = GeoCoord::NIL;

  protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;
    virtual void orient() override;

    /**
     * Reads the configured TLE file, selects the satellite by index, name, or NORAD
     * catalog number, and initializes the SGP4 element record (satrec). Also writes
     * the resolved satellite name back to the 'satelliteName' parameter.
     */
    virtual void loadTleRecord();

    /** Propagates the orbit and returns the scene position at the given UTC Julian date. */
    virtual Coord computeScenePosition(double julianDate, GeoCoord& geographicPosition);

  public:
    virtual double getMaxSpeed() const override { return maxSpeed; }

    /** Returns the cached geodetic (latitude/longitude/altitude) sub-satellite position. */
    virtual const GeoCoord& getGeographicPosition() const { return lastGeoPosition; }
};

} // namespace inet


#endif

