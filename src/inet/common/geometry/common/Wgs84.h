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
 * satellite mobility, the geographic anchor mobility, and the satellite
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
const double SEMI_MAJOR_AXIS = 6378137.0;
/** WGS84 flattening. */
const double FLATTENING = 1.0 / 298.257223563;
/** WGS84 semi-minor axis (polar radius), meters. */
const double SEMI_MINOR_AXIS = SEMI_MAJOR_AXIS * (1.0 - FLATTENING);
/** First eccentricity squared. */
const double ECCENTRICITY_SQUARED = FLATTENING * (2.0 - FLATTENING);
/** Mean Earth radius (IUGG arithmetic mean (2a+b)/3), meters; used for spherical great-circle math. */
const double MEAN_RADIUS = (2.0 * SEMI_MAJOR_AXIS + SEMI_MINOR_AXIS) / 3.0;

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
 * Returns the rotation (as a quaternion) that maps a vector expressed in the inertial
 * TEME (ECI) frame into the ECEF frame at the given GMST angle. This is the rotational
 * equivalent of eciTemeToEcef(): for any vector v,
 * eciTemeToEcefRotation(gmstRad).rotate(v) == eciTemeToEcef(v, gmstRad).
 */
INET_API Quaternion eciTemeToEcefRotation(double gmstRad);

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

/**
 * Returns the great-circle (orthodromic) surface distance in meters between two
 * geographic positions, computed with the haversine formula on a sphere of
 * meanRadius. Altitudes are ignored.
 */
INET_API double greatCircleDistance(const GeoCoord& a, const GeoCoord& b);

/**
 * Interpolates along the great-circle arc between two geographic positions.
 * The latitude/longitude follow the shortest great circle (spherical linear
 * interpolation of the unit direction vectors), so the path stays on the surface
 * even for arcs long enough that a straight line in Cartesian space would cut
 * through the Earth; the altitude is interpolated linearly. The parameter t is
 * the fraction of the arc in [0, 1] (t=0 returns a, t=1 returns b).
 *
 * This is a spherical approximation: the latitude is treated as geocentric rather
 * than geodetic, so an intermediate latitude can differ from the true geodetic value
 * by up to ~0.2 deg (~20 km) when the endpoints are far apart in latitude. Coincident
 * endpoints return that point; antipodal endpoints have no unique great circle and are
 * swept along an arbitrary meridian.
 */
INET_API GeoCoord interpolateGreatCircle(const GeoCoord& a, const GeoCoord& b, double t);

} // namespace wgs84

} // namespace inet

#endif

