//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_QUATERNION_H
#define __INET_QUATERNION_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"

namespace inet {

// borrowed and adapted from https://github.com/MegaManSE/willperone/blob/master/Math/quaternion.h
class INET_API Quaternion
{
public:

    double s = 1; //!< the real component
    Coord v; //!< the imaginary components

    //! basic ctors
    Quaternion() {}
    Quaternion(double real, double i, double j, double k): s(real), v(i, j, k) { }
    Quaternion(double real, const Coord &imag): s(real), v(imag) { }

    //! constructs a Quaternion from a normalized axis - angle pair rotation
    Quaternion(const Coord &axis, double angle);

    //! from 3 euler angles
    explicit Quaternion(const EulerAngles& angles);

    //! basic operations
    Quaternion &operator =(const Quaternion &q) { s = q.s; v = q.v; return *this; }

    const Quaternion operator +(const Quaternion &q) const { return Quaternion(s+q.s, v+q.v); }
    const Quaternion operator -(const Quaternion &q) const { return Quaternion(s-q.s, v-q.v); }
    const Quaternion operator *(const Quaternion &q) const;
    const Quaternion operator /(const Quaternion &q) const { return (*this) * q.inverse(); }

    const Quaternion operator *(double scale) const { return Quaternion(s * scale,v * scale); }
    const Quaternion operator /(double scale) const { return Quaternion(s / scale, v / scale); }

    const Quaternion operator -() const { return Quaternion(-s, -v); }

    const Quaternion &operator +=(const Quaternion &q) { v += q.v; s += q.s; return *this; }
    const Quaternion &operator -=(const Quaternion &q) { v -= q.v; s -= q.s; return *this; }
    const Quaternion &operator *=(const Quaternion &q);

    const Quaternion &operator *= (double scale) { s *= scale; v *= scale; return *this; }
    const Quaternion &operator /= (double scale) { s /= scale; v /= scale; return *this; }


    //! gets the length of this Quaternion
    double length() const { return std::sqrt(s*s + v*v); }

    //! gets the squared length of this Quaternion
    double lengthSquared() const { return (s*s + v*v); }

    //! normalizes this Quaternion
    void normalize() { *this /= length(); }

    //! returns the normalized version of this Quaternion
    Quaternion normalized() const { return (*this) / length(); }

    //! computes the conjugate of this Quaternion
    void conjugate() { v = -v; }

    //! inverts this Quaternion
    void invert() { conjugate(); *this /= lengthSquared(); }

    //! returns the inverse of this Quaternion
    Quaternion inverse() const { Quaternion q(*this); q.invert(); return q; }


    //! computes the dot product of 2 quaternions
    static inline double dot(const Quaternion &q1, const Quaternion &q2) { return q1.v*q2.v + q1.s*q2.s; }

    //! linear quaternion interpolation
    static Quaternion lerp(const Quaternion &q1, const Quaternion &q2, double t) { return (q1*(1-t) + q2*t).normalized(); }

    //! spherical linear interpolation
    static Quaternion slerp(const Quaternion &q1, const Quaternion &q2, double t);

    //! returns the axis and angle of this unit Quaternion
    void toAxisAngle(Coord &axis, double &angle) const;

    //! rotates v by this quaternion (quaternion must be unit)
    Coord rotate(const Coord &v);

    //! returns the euler angles from a rotation Quaternion
    EulerAngles toEulerAngles(bool homogenous=false) const;

    // adapted from https://svn.code.sf.net/p/irrlicht/code/trunk/include/quaternion.h
    static Quaternion rotationFromTo(const Coord& from, const Coord& to);
};



} /* namespace inet */

#endif // ifndef __INET_QUATERNION_H
