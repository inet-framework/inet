/***************************************************************************
 * file:        Coord.h
 *
 * author:      Christian Frank
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef __INET_COORD_H
#define __INET_COORD_H

#include "INETDefs.h"

#include "FWMath.h"


/**
 * @brief Class for storing 3D coordinates.
 *
 * Some comparison and basic arithmetic operators are implemented.
 *
 * @ingroup utils
 * @author Christian Frank
 */
class INET_API Coord : public cObject
{
public:
    /** @brief Constant with all values set to 0. */
    static const Coord ZERO;

public:
    /** @name x, y and z coordinate of the position. */
    /*@{*/
    double x;
    double y;
    double z;
    /*@}*/

private:
  void copy(const Coord& other) { x = other.x; y = other.y; z = other.z; }

public:
    /** @brief Default constructor. */
    Coord()
        : x(0.0), y(0.0), z(0.0) {}

    /** @brief Initializes a coordinate. */
    Coord(double x, double y, double z = 0.0)
        : x(x), y(y), z(z) {}

    /** @brief Initializes coordinate from other coordinate. */
    Coord(const Coord& other)
        : cObject(other) { copy(other); }

    /** @brief Returns a string with the value of the coordinate. */
    std::string info() const {
        std::stringstream os;
        os << this;
        return os.str();
    }

    /** @brief Adds two coordinate vectors. */
    friend Coord operator+(const Coord& a, const Coord& b) {
        Coord tmp(a);
        tmp += b;
        return tmp;
    }

    /** @brief Subtracts two coordinate vectors. */
    friend Coord operator-(const Coord& a, const Coord& b) {
        Coord tmp(a);
        tmp -= b;
        return tmp;
    }

    /** @brief Multiplies a coordinate vector by a real number. */
    friend Coord operator*(const Coord& a, double f) {
        Coord tmp(a);
        tmp *= f;
        return tmp;
    }

    /** @brief Divides a coordinate vector by a real number. */
    friend Coord operator/(const Coord& a, double f) {
        Coord tmp(a);
        tmp /= f;
        return tmp;
    }

    /**
     * @brief Multiplies this coordinate vector by a real number.
     */
    Coord& operator*=(double f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    /**
     * @brief Divides this coordinate vector by a real number.
     */
    Coord& operator/=(double f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    /**
     * @brief Adds coordinate vector 'a' to this.
     */
    Coord& operator+=(const Coord& a) {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    /**
     * @brief Assigns coordinate vector 'other' to this.
     *
     * This operator can change the dimension of the coordinate.
     */
    Coord& operator=(const Coord& other) {
        if (this == &other) return *this;
        cObject::operator=(other);
        copy(other);
        return *this;
    }

    /**
     * @brief Subtracts coordinate vector 'a' from this.
     */
    Coord& operator-=(const Coord& a) {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }

    /**
     * @brief Tests whether two coordinate vectors are equal.
     *
     * Because coordinates are of type double, this is done through the
     * FWMath::close function.
     */
    friend bool operator==(const Coord& a, const Coord& b) {
        // FIXME: this implementation is not transitive
        return FWMath::close(a.x, b.x) && FWMath::close(a.y, b.y) && FWMath::close(a.z, b.z);
    }

    /**
     * @brief Tests whether two coordinate vectors are not equal.
     *
     * Negation of the operator==.
     */
    friend bool operator!=(const Coord& a, const Coord& b) {
        return !(a==b);
    }

    /**
     * @brief Returns the distance to Coord 'a'.
     */
    double distance(const Coord& a) const {
        Coord dist(*this - a);
        return dist.length();
    }

    /**
     * @brief Returns distance^2 to Coord 'a' (omits calling square root).
     */
    double sqrdist(const Coord& a) const {
        Coord dist(*this - a);
        return dist.squareLength();
    }

    /**
     * @brief Returns the squared distance on a torus of this to Coord 'b' (omits calling square root).
     */
    double sqrTorusDist(const Coord& b, const Coord& size) const;

    /**
     * @brief Returns the square of the length of this Coords position vector.
     */
    double squareLength() const
    {
        return x * x + y * y + z * z;
    }

    /**
     * @brief Returns the length of this Coords position vector.
     */
    double length() const
    {
        return sqrt(squareLength());
    }

    /**
     * @brief Checks if this coordinate is inside a specified rectangle.
     *
     * @param lowerBound The upper bound of the rectangle.
     * @param upperBound The lower bound of the rectangle.
     */
    bool isInBoundary(const Coord& lowerBound, const Coord& upperBound) const {
        return  lowerBound.x <= x && x <= upperBound.x &&
                lowerBound.y <= y && y <= upperBound.y &&
                lowerBound.z <= z && z <= upperBound.z;
    }

    /**
     * @brief Returns the minimal coordinates.
     */
    Coord min(const Coord& a) {
        return Coord(this->x < a.x ? this->x : a.x,
                     this->y < a.y ? this->y : a.y,
                     this->z < a.z ? this->z : a.z);
    }

    /**
     * @brief Returns the maximal coordinates.
     */
    Coord max(const Coord& a) {
        return Coord(this->x > a.x ? this->x : a.x,
                     this->y > a.y ? this->y : a.y,
                     this->z > a.z ? this->z : a.z);
    }
};


inline std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    return os << "(" << coord.x << "," << coord.y << "," << coord.z << ")";
}

#endif
