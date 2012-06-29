#include <cmath>

#include <gnplib/impl/network/gnp/HaversineHelpers.h>

namespace HaversineHelpers=gnplib::impl::network::gnp::HaversineHelpers;

double HaversineHelpers::radians(double degrees)
{
    return degrees*(2*M_PI)/360;
}

double HaversineHelpers::degrees(double radians)
{
    return radians*360/(2*M_PI);
}

double HaversineHelpers::square(double d)
{
    return d * d;
}

/**
 * Computes the earth's radius of curvature at a particular latitude,
 * assuming that the earth is a squashed sphere with elliptical
 * cross-section.
 *
 * @param lat -
 *            latitude in radians. This is the angle that a point at this
 *            latitude makes with the horizontal.
 */
double HaversineHelpers::globeRadiusOfCurvature(double lat)
{
    double a=GLOBE_RADIUS_EQUATOR; // major axis
    double b=GLOBE_RADIUS_POLES; // minor axis
    double e=sqrt(1-square(b/a)); // eccentricity
    return a*sqrt(1-square(e))/(1-square(e*sin(lat)));
}
