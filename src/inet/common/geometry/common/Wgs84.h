//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_WGS84_H
#define __INET_WGS84_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

/**
 * Astronomy and geodesy helpers shared by the geocentric coordinate system, the
 * satellite mobility, the geographic anchor mobility, and (later) the satellite
 * visualizers. All conversions use the WGS84 ellipsoid and the meter as the unit
 * for Cartesian coordinates.
 *
 * The Earth-orientation conversion is GMST-only: nutation, polar motion and the
 * UT1-UTC difference are ignored (UT1 is approximated by UTC). This yields a
 * ground-track error on the order of tens of meters, which is adequate for
 * network simulation but not for precision orbit determination.
 */
namespace wgs84 {

/** WGS84 semi-major axis (equatorial radius), meters. */
const double semiMajorAxis = 6378137.0;
/** WGS84 flattening. */
const double flattening = 1.0 / 298.257223563;
/** WGS84 semi-minor axis (polar radius), meters. */
const double semiMinorAxis = semiMajorAxis * (1.0 - flattening);
/** First eccentricity squared. */
const double eccentricitySquared = flattening * (2.0 - flattening);

/** Topocentric look angles from an observer to a target. */
class INET_API LookAngles
{
  public:
    rad azimuth; ///< measured clockwise from local north
    rad elevation; ///< measured up from the local horizontal plane
    m range; ///< straight-line distance to the target

    LookAngles(rad azimuth, rad elevation, m range) : azimuth(azimuth), elevation(elevation), range(range) {}
};

/**
 * Converts WGS84 geodetic coordinates (latitude, longitude, ellipsoidal height)
 * to Earth-Centered Earth-Fixed (ECEF) Cartesian coordinates in meters.
 */
INET_API Coord geodeticToEcef(const GeoCoord& geographicCoordinate);

/**
 * Converts ECEF Cartesian coordinates (meters) to WGS84 geodetic coordinates
 * using Bowring's closed-form method.
 */
INET_API GeoCoord ecefToGeodetic(const Coord& ecefCoordinate);

/**
 * Returns the Julian date corresponding to the given UTC calendar date and time.
 * Follows Vallado's jday() algorithm.
 */
INET_API double julianDateFromUtc(int year, int month, int day, int hour, int minute, double second);

/**
 * Returns the Greenwich Mean Sidereal Time (radians, in [0, 2*pi)) for the given
 * Julian date (UT1 approximated by UTC). Follows Vallado's gstime() algorithm.
 */
INET_API double gmst(double julianDate);

/**
 * Rotates a vector from the TEME (an Earth-Centered Inertial frame produced by
 * SGP4) into the ECEF frame, given the Greenwich sidereal angle in radians.
 * Works for both position and an instantaneous direction; it does not add the
 * Earth-rotation (omega x r) term, so velocities computed with it are inertial
 * velocities expressed in ECEF axes.
 */
INET_API Coord eciTemeToEcef(const Coord& teme, double gmstRad);

/**
 * Rotates a vector given in the local East-North-Up frame at the given geographic
 * anchor into the ECEF frame (rotation only, no translation).
 */
INET_API Coord enuVectorToEcef(const GeoCoord& anchor, const Coord& enu);

/**
 * Returns the rotation (as a quaternion) that maps a vector expressed in the local
 * East-North-Up frame at the given geographic anchor into the ECEF frame. This is
 * the rotational equivalent of enuVectorToEcef(): for any vector v,
 * enuToEcefRotation(anchor).rotate(v) == enuVectorToEcef(anchor, v).
 */
INET_API Quaternion enuToEcefRotation(const GeoCoord& anchor);

/**
 * Computes the topocentric azimuth, elevation and range of a target (given in
 * ECEF) as seen from an observer at the given geographic position.
 */
INET_API LookAngles computeLookAngles(const GeoCoord& observer, const Coord& targetEcef);

} // namespace wgs84

} // namespace inet

#endif

