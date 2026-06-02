//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/Wgs84.h"

#include <cmath>

namespace inet {

namespace wgs84 {

Coord geodeticToEcef(const GeoCoord& geographicCoordinate)
{
    double lat = geographicCoordinate.latitude.get<rad>();
    double lon = geographicCoordinate.longitude.get<rad>();
    double alt = geographicCoordinate.altitude.get<m>();
    double sinLat = sin(lat);
    double cosLat = cos(lat);
    double n = semiMajorAxis / sqrt(1.0 - eccentricitySquared * sinLat * sinLat);
    double x = (n + alt) * cosLat * cos(lon);
    double y = (n + alt) * cosLat * sin(lon);
    double z = (n * (1.0 - eccentricitySquared) + alt) * sinLat;
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
        double alt = std::abs(z) - semiMinorAxis;
        return GeoCoord(rad(lat), rad(lon), m(alt));
    }
    double secondEccentricitySquared = (semiMajorAxis * semiMajorAxis - semiMinorAxis * semiMinorAxis) / (semiMinorAxis * semiMinorAxis);
    double theta = atan2(z * semiMajorAxis, p * semiMinorAxis);
    double sinTheta = sin(theta);
    double cosTheta = cos(theta);
    double lat = atan2(z + secondEccentricitySquared * semiMinorAxis * sinTheta * sinTheta * sinTheta,
            p - eccentricitySquared * semiMajorAxis * cosTheta * cosTheta * cosTheta);
    double sinLat = sin(lat);
    double n = semiMajorAxis / sqrt(1.0 - eccentricitySquared * sinLat * sinLat);
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
    double r00 = -sinLon, r01 = -sinLat * cosLon, r02 = cosLat * cosLon;
    double r10 = cosLon, r11 = -sinLat * sinLon, r12 = cosLat * sinLon;
    double r20 = 0.0, r21 = cosLat, r22 = sinLat;
    // standard rotation matrix -> quaternion conversion
    double trace = r00 + r11 + r22;
    double w, x, y, z;
    if (trace > 0.0) {
        double k = 0.5 / sqrt(trace + 1.0);
        w = 0.25 / k;
        x = (r21 - r12) * k;
        y = (r02 - r20) * k;
        z = (r10 - r01) * k;
    }
    else if (r00 > r11 && r00 > r22) {
        double k = 2.0 * sqrt(1.0 + r00 - r11 - r22);
        w = (r21 - r12) / k;
        x = 0.25 * k;
        y = (r01 + r10) / k;
        z = (r02 + r20) / k;
    }
    else if (r11 > r22) {
        double k = 2.0 * sqrt(1.0 + r11 - r00 - r22);
        w = (r02 - r20) / k;
        x = (r01 + r10) / k;
        y = 0.25 * k;
        z = (r12 + r21) / k;
    }
    else {
        double k = 2.0 * sqrt(1.0 + r22 - r00 - r11);
        w = (r10 - r01) / k;
        x = (r02 + r20) / k;
        y = (r12 + r21) / k;
        z = 0.25 * k;
    }
    return Quaternion(w, x, y, z).normalized();
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

} // namespace wgs84

} // namespace inet

