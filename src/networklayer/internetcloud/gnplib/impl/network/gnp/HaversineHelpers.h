#ifndef __GNPLIB_IMPL_NETWORK_GNP_HAVERSINEHELPERS_H
#define __GNPLIB_IMPL_NETWORK_GNP_HAVERSINEHELPERS_H

namespace gnplib { namespace impl { namespace network { namespace gnp {

namespace HaversineHelpers
{

/** Radius of the earth, in meters, at the equator. */
const double GLOBE_RADIUS_EQUATOR(6378000);

/** Radius of the earth, in meters, at the poles. */
const double GLOBE_RADIUS_POLES(6357000);

double radians(double degrees);

double degrees(double radians);

double square(double d);

/**
 * Computes the earth's radius of curvature at a particular latitude,
 * assuming that the earth is a squashed sphere with elliptical
 * cross-section.
 *
 * @param lat -
 *            latitude in radians. This is the angle that a point at this
 *            latitude makes with the horizontal.
 */
double globeRadiusOfCurvature(double lat);

}; // namespace HaversineHelpers

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_HAVERSINEHELPERS_H
