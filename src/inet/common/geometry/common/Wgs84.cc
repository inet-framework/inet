//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/Wgs84.h"

#include <cmath>

#include "inet/common/geometry/common/RotationMatrix.h"

namespace inet {

namespace wgs84 {

Coord geodeticToEcef(const GeoCoord& geographicCoordinate)
{
    double lat = geographicCoordinate.latitude.get<rad>();
    double lon = geographicCoordinate.longitude.get<rad>();
    double alt = geographicCoordinate.altitude.get<m>();
    double sinLat = sin(lat);
    double cosLat = cos(lat);
    double n = SEMI_MAJOR_AXIS / sqrt(1.0 - ECCENTRICITY_SQUARED * sinLat * sinLat);
    double x = (n + alt) * cosLat * cos(lon);
    double y = (n + alt) * cosLat * sin(lon);
    double z = (n * (1.0 - ECCENTRICITY_SQUARED) + alt) * sinLat;
    return Coord(x, y, z);
}

GeoCoord ecefToGeodetic(const Coord& ecefCoordinate)
{
    double x = ecefCoordinate.x;
    double y = ecefCoordinate.y;
    double z = ecefCoordinate.z;
    double lon = atan2(y, x);
    double p = sqrt(x * x + y * y);
    if (p < 1e-9) {
        // on the polar axis
        double lat = (z >= 0 ? M_PI / 2 : -M_PI / 2);
        double alt = std::abs(z) - SEMI_MINOR_AXIS;
        return GeoCoord(rad(lat), rad(lon), m(alt));
    }
    double secondEccentricitySquared = (SEMI_MAJOR_AXIS * SEMI_MAJOR_AXIS - SEMI_MINOR_AXIS * SEMI_MINOR_AXIS) / (SEMI_MINOR_AXIS * SEMI_MINOR_AXIS);
    double theta = atan2(z * SEMI_MAJOR_AXIS, p * SEMI_MINOR_AXIS);
    double sinTheta = sin(theta);
    double cosTheta = cos(theta);
    double lat = atan2(z + secondEccentricitySquared * SEMI_MINOR_AXIS * sinTheta * sinTheta * sinTheta,
            p - ECCENTRICITY_SQUARED * SEMI_MAJOR_AXIS * cosTheta * cosTheta * cosTheta);
    double sinLat = sin(lat);
    double n = SEMI_MAJOR_AXIS / sqrt(1.0 - ECCENTRICITY_SQUARED * sinLat * sinLat);
    double alt = p / cos(lat) - n;
    return GeoCoord(rad(lat), rad(lon), m(alt));
}

double julianDateFromUtc(int year, int month, int day, int hour, int minute, double second)
{
    double jd = 367.0 * year
        - floor((7 * (year + floor((month + 9) / 12.0))) * 0.25)
        + floor(275 * month / 9.0)
        + day + 1721013.5;
    double jdFrac = (second + minute * 60.0 + hour * 3600.0) / 86400.0;
    return jd + jdFrac;
}

double gmst(double julianDate)
{
    double tut1 = (julianDate - 2451545.0) / 36525.0;
    // Vallado gstime(): result in seconds of time, then converted to radians
    double temp = -6.2e-6 * tut1 * tut1 * tut1
        + 0.093104 * tut1 * tut1
        + (876600.0 * 3600.0 + 8640184.812866) * tut1
        + 67310.54841;
    // 360/86400 deg per second of time, then deg -> rad: (M_PI/180)/240
    temp = fmod(temp * (M_PI / 180.0 / 240.0), 2.0 * M_PI);
    if (temp < 0.0)
        temp += 2.0 * M_PI;
    return temp;
}

Coord eciTemeToEcef(const Coord& teme, double gmstRad)
{
    double c = cos(gmstRad);
    double s = sin(gmstRad);
    // r_ecef = R3(gmst) * r_eci
    return Coord(c * teme.x + s * teme.y,
            -s * teme.x + c * teme.y,
            teme.z);
}

Quaternion eciTemeToEcefRotation(double gmstRad)
{
    // eciTemeToEcef() is R3(gmst), i.e. a rotation about +Z by -gmst (it maps the ECI +X axis to
    // (cos gmst, -sin gmst, 0) in ECEF); the axis-angle quaternion below reproduces it exactly.
    return Quaternion(Coord::Z_AXIS, -gmstRad);
}

Coord enuVectorToEcef(const GeoCoord& anchor, const Coord& enu)
{
    double lat = anchor.latitude.get<rad>();
    double lon = anchor.longitude.get<rad>();
    double sinLat = sin(lat), cosLat = cos(lat);
    double sinLon = sin(lon), cosLon = cos(lon);
    double e = enu.x, n = enu.y, u = enu.z;
    double x = -sinLon * e - sinLat * cosLon * n + cosLat * cosLon * u;
    double y = cosLon * e - sinLat * sinLon * n + cosLat * sinLon * u;
    double z = cosLat * n + sinLat * u;
    return Coord(x, y, z);
}

Quaternion enuToEcefRotation(const GeoCoord& anchor)
{
    double lat = anchor.latitude.get<rad>();
    double lon = anchor.longitude.get<rad>();
    double sinLat = sin(lat), cosLat = cos(lat);
    double sinLon = sin(lon), cosLon = cos(lon);
    // rotation matrix whose columns are the East, North, Up unit vectors in ECEF
    double matrix[3][3] = {
        { -sinLon, -sinLat * cosLon, cosLat * cosLon },
        {  cosLon, -sinLat * sinLon, cosLat * sinLon },
        {     0.0,           cosLat,         sinLat   },
    };
    return RotationMatrix(matrix).toQuaternion().normalized();
}

LookAngles computeLookAngles(const GeoCoord& observer, const Coord& targetEcef)
{
    Coord observerEcef = geodeticToEcef(observer);
    Coord d = targetEcef - observerEcef;
    double lat = observer.latitude.get<rad>();
    double lon = observer.longitude.get<rad>();
    double sinLat = sin(lat), cosLat = cos(lat);
    double sinLon = sin(lon), cosLon = cos(lon);
    // rotate the ECEF difference vector into the observer's ENU frame
    double e = -sinLon * d.x + cosLon * d.y;
    double n = -sinLat * cosLon * d.x - sinLat * sinLon * d.y + cosLat * d.z;
    double u = cosLat * cosLon * d.x + cosLat * sinLon * d.y + sinLat * d.z;
    double range = d.length();
    double elevation = (range > 0 ? asin(u / range) : 0.0);
    double azimuth = atan2(e, n);
    if (azimuth < 0.0)
        azimuth += 2.0 * M_PI;
    return LookAngles(rad(azimuth), rad(elevation), m(range));
}

double greatCircleDistance(const GeoCoord& a, const GeoCoord& b)
{
    double lat1 = a.latitude.get<rad>();
    double lat2 = b.latitude.get<rad>();
    double dLat = lat2 - lat1;
    double dLon = b.longitude.get<rad>() - a.longitude.get<rad>();
    double sinHalfLat = sin(dLat / 2.0);
    double sinHalfLon = sin(dLon / 2.0);
    double h = sinHalfLat * sinHalfLat + cos(lat1) * cos(lat2) * sinHalfLon * sinHalfLon;
    double c = 2.0 * atan2(sqrt(h), sqrt(std::max(0.0, 1.0 - h)));
    return MEAN_RADIUS * c;
}

GeoCoord interpolateGreatCircle(const GeoCoord& a, const GeoCoord& b, double t)
{
    double lat1 = a.latitude.get<rad>(), lon1 = a.longitude.get<rad>();
    double lat2 = b.latitude.get<rad>(), lon2 = b.longitude.get<rad>();
    // unit direction vectors on the sphere (latitude/longitude treated as geocentric here)
    Coord v1(cos(lat1) * cos(lon1), cos(lat1) * sin(lon1), sin(lat1));
    Coord v2(cos(lat2) * cos(lon2), cos(lat2) * sin(lon2), sin(lat2));
    double alt = a.altitude.get<m>() * (1.0 - t) + b.altitude.get<m>() * t;
    double dot = std::max(-1.0, std::min(1.0, v1 * v2));
    double omega = acos(dot);
    double sinOmega = sin(omega);
    Coord v;
    if (sinOmega < 1e-9) {
        if (dot > 0)
            // coincident endpoints: the linear blend is well-conditioned (v1 ~ v2)
            v = (v1 * (1.0 - t) + v2 * t);
        else {
            // antipodal endpoints: the great circle is not unique; sweep 180 deg from v1 around an
            // arbitrary axis perpendicular to it (an arbitrary meridian) so the motion stays continuous
            Coord ref = std::fabs(v1.z) < 0.9 ? Coord(0, 0, 1) : Coord(1, 0, 0);
            Coord axis = v1 % ref; axis.normalize();
            Coord perp = axis % v1; // unit, perpendicular to v1, in the rotation plane
            double angle = t * M_PI;
            v = v1 * cos(angle) + perp * sin(angle);
        }
    }
    else {
        double k1 = sin((1.0 - t) * omega) / sinOmega;
        double k2 = sin(t * omega) / sinOmega;
        v = v1 * k1 + v2 * k2;
    }
    double len = v.length();
    if (len < 1e-12)
        return GeoCoord(rad(lat1), rad(lon1), m(alt));
    v /= len;
    return GeoCoord(rad(asin(std::max(-1.0, std::min(1.0, v.z)))), rad(atan2(v.y, v.x)), m(alt));
}

} // namespace wgs84

} // namespace inet

