//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

const Quaternion Quaternion::IDENTITY = Quaternion();
const Quaternion Quaternion::NIL = Quaternion(NaN, NaN, NaN, NaN);

Quaternion::Quaternion(const Coord& axis, double angle) : Quaternion(std::cos(angle / 2), axis * std::sin(angle / 2))
{
    // nothing
}

Quaternion::Quaternion(const EulerAngles& angles)
{
    double cos_z_2 = std::cos(0.5 * rad(angles.alpha).get());
    double cos_y_2 = std::cos(0.5 * rad(angles.beta).get());
    double cos_x_2 = std::cos(0.5 * rad(angles.gamma).get());

    double sin_z_2 = std::sin(0.5 * rad(angles.alpha).get());
    double sin_y_2 = std::sin(0.5 * rad(angles.beta).get());
    double sin_x_2 = std::sin(0.5 * rad(angles.gamma).get());

    // and now compute Quaternion
    s = cos_z_2 * cos_y_2 * cos_x_2 + sin_z_2 * sin_y_2 * sin_x_2;
    v.x = cos_z_2 * cos_y_2 * sin_x_2 - sin_z_2 * sin_y_2 * cos_x_2;
    v.y = cos_z_2 * sin_y_2 * cos_x_2 + sin_z_2 * cos_y_2 * sin_x_2;
    v.z = sin_z_2 * cos_y_2 * cos_x_2 - cos_z_2 * sin_y_2 * sin_x_2;
}

const Quaternion Quaternion::operator*(const Quaternion& q) const
{
    return Quaternion(s * q.s - v * q.v,
                      v.y * q.v.z - v.z * q.v.y + s * q.v.x + v.x * q.s,
                      v.z * q.v.x - v.x * q.v.z + s * q.v.y + v.y * q.s,
                      v.x * q.v.y - v.y * q.v.x + s * q.v.z + v.z * q.s);
}

const Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    double x = v.x, y = v.y, z = v.z, sn = s * q.s - v * q.v;

    v.x = y * q.v.z - z * q.v.y + s * q.v.x + x * q.s;
    v.y = z * q.v.x - x * q.v.z + s * q.v.y + y * q.s;
    v.z = x * q.v.y - y * q.v.x + s * q.v.z + z * q.s;

    s = sn;

    return *this;
}

/*
//! casting to a 4x4 isomorphic matrix for right multiplication with vector
operator matrix4() const
{
    return matrix4(s,  -v.x, -v.y,-v.z,
                   v.x,  s,  -v.z, v.y,
                   v.y, v.z,    s,-v.x,
                   v.z,-v.y,  v.x,   s);
}
*/

//! casting to a 4x4 isomorphic matrix for left multiplication with vector
/*operator matrix4() const
{
    return matrix4(   s, v.x, v.y, v.z,
                   -v.x,   s,-v.z, v.y,
                   -v.y, v.z,   s,-v.x,
                   -v.z,-v.y, v.x,   s);
}*/

/*
//! casting to 3x3 rotation matrix
operator matrix3() const
{
    Assert(length() > 0.9999 && length() < 1.0001, "quaternion is not normalized");
    return matrix3(1-2*(v.y*v.y+v.z*v.z), 2*(v.x*v.y-s*v.z),   2*(v.x*v.z+s*v.y),
                   2*(v.x*v.y+s*v.z),   1-2*(v.x*v.x+v.z*v.z), 2*(v.y*v.z-s*v.x),
                   2*(v.x*v.z-s*v.y),   2*(v.y*v.z+s*v.x),   1-2*(v.x*v.x+v.y*v.y));
}
*/

Quaternion Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, double t)
{
    Quaternion q3;
    double dot = Quaternion::dot(q1, q2);

    /*  dot = cos(theta)
        if (dot < 0), q1 and q2 are more than 90 degrees apart,
        so we can invert one to reduce spinning */
    if (dot < 0) {
        dot = -dot;
        q3 = -q2;
    }
    else q3 = q2;

    if (dot < 0.95) {
        double angle = std::acos(dot);
        return (q1 * std::sin(angle * (1 - t)) + q3 * std::sin(angle * t)) / std::sin(angle);
    }
    else // if the angle is small, use linear interpolation
        return lerp(q1, q3, t);
}

void Quaternion::getRotationAxisAndAngle(Coord& axis, double& angle) const
{
    angle = std::acos(s);

    // pre-compute to save time
    double sin_theta_inv = 1.0 / std::sin(angle);

    // now the vector
    axis.x = v.x * sin_theta_inv;
    axis.y = v.y * sin_theta_inv;
    axis.z = v.z * sin_theta_inv;

    angle *= 2;
}

Coord Quaternion::rotate(const Coord& v) const
{
    Quaternion V(0, v);
    Quaternion conjugate(*this);
    conjugate.conjugate();
    return (*this * V * conjugate).v;
}

EulerAngles Quaternion::toEulerAngles(bool homogenous) const
{
    // NOTE: this algorithm is prone to gimbal lock when beta is close to +-90
    double sqw = s * s;
    double sqx = v.x * v.x;
    double sqy = v.y * v.y;
    double sqz = v.z * v.z;

    EulerAngles euler;
    if (homogenous) {
        euler.gamma = rad(std::atan2(2.0 * (v.x * v.y + v.z * s), sqx - sqy - sqz + sqw));
        euler.beta = rad(std::asin(math::minnan(1.0, math::maxnan(-1.0, -2.0 * (v.x * v.z - v.y * s)))));
        euler.alpha = rad(std::atan2(2.0 * (v.y * v.z + v.x * s), -sqx - sqy + sqz + sqw));
    }
    else {
        euler.gamma = rad(std::atan2(2.0 * (v.z * v.y + v.x * s), 1 - 2 * (sqx + sqy)));
        euler.beta = rad(std::asin(math::minnan(1.0, math::maxnan(-1.0, -2.0 * (v.x * v.z - v.y * s)))));
        euler.alpha = rad(std::atan2(2.0 * (v.x * v.y + v.z * s), 1 - 2 * (sqy + sqz)));
    }
    euler.normalize();
    return euler;
}

Quaternion Quaternion::rotationFromTo(const Coord& from, const Coord& to)
{
    // Based on Stan Melax's article in Game Programming Gems
    // Copy, since cannot modify local
    Coord v0 = from;
    Coord v1 = to;
    v0.normalize();
    v1.normalize();

    double d = v0 * v1;

    if (d >= 1.0f) { // If dot == 1, vectors are the same
        return Quaternion(1, 0, 0, 0); // identity
    }
    else if (d <= -1.0f) { // exactly opposite
        Coord axis = Coord(1, 0, 0) % v0;

        if (axis.length() == 0)
            axis = Coord(0, 1, 0) % v0;

        // same as fromAxisAngle(axis, PI).normalize();
        return Quaternion(0, axis.x, axis.y, axis.z).normalized();
    }

    double s = std::sqrt((1 + d) * 2);

    Coord c = (v0 % v1) / s;

    return Quaternion(s * 0.5, c.x, c.y, c.z).normalized();
}

void Quaternion::getSwingAndTwist(const Coord& direction, Quaternion& swing, Quaternion& twist) const
{
    Coord p = direction * (v * direction); // return projection v1 on to v2  (parallel component)
    twist = Quaternion(s, p);
    twist.normalize();
    swing = *this * twist.conjugated();
}

} /* namespace inet */

