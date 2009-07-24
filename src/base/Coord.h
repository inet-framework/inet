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

#include <omnetpp.h>
#include "INETDefs.h"
#include "FWMath.h"

/**
 * @brief Class for storing host positions
 *
 * Class for a double-tuple storing a position / two-dimensional
 * vector. Some comparison and basic arithmetic operators on Coord
 * structures are implemented.
 *
 * @ingroup support
 * @author Christian Frank
 */
class INET_API Coord : public cPolymorphic
{
 public:
  /** @brief x and y coordinates of the position*/
  double x,y;

  /** Initializes coordinates.*/
  Coord(double _x=0, double _y=0) : x(_x), y(_y) {};


  /** Initializes coordinates.*/
  Coord(const Coord& pos) {
        x=pos.x;
        y=pos.y;
  }

  /** Initializes coordinates.*/
  Coord(const Coord* pos) {
        x=pos->x;
        y=pos->y;
  }

  std::string info() const {
        std::stringstream os;
        os << "(" << x << "," << y << ")";
        return os.str();
  }

  /** Adds two coordinate vectors.*/
  friend Coord operator+(const Coord& a, const Coord& b) {
        return Coord(a.x+b.x, a.y+b.y);
  }

  /** Subtracts two coordinate vectors.*/
  friend Coord operator-(const Coord& a, const Coord& b) {
        return Coord(a.x-b.x, a.y-b.y);
  }

  /** Multiplies a coordinate vector by a real number.*/
  friend Coord operator*(const Coord& a, double f) {
        return Coord(a.x*f, a.y*f);
  }

  /** Divides a coordinate vector by a real number.*/
  friend Coord operator/(const Coord& a, double f) {
        return Coord(a.x/f, a.y/f);
  }

  /** Adds coordinate vector b to a.*/
  const Coord& operator+=(const Coord& a) {
        x+=a.x;
        y+=a.y;
        return *this;
  }

  /** Assigns a this.*/
  const Coord& operator=(const Coord& a) {
        x=a.x;
        y=a.y;
        return *this;
  }

  /** Subtracts coordinate vector b from a.*/
  const Coord& operator-=(const Coord& a) {
        x-=a.x;
        y-=a.y;
        return *this;
  }

  /**
   * Tests whether two coordinate vectors are equal. Because
   * coordinates are of type double, this is done through the
   * FWMath::close function.
   */
  friend bool operator==(const Coord& a, const Coord& b) {
        return FWMath::close(a.x,b.x) && FWMath::close(a.y,b.y);
  }

  /**
   * Tests whether two coordinate vectors are not equal. Negation of
   * the operator==.
   */
  friend bool operator!=(const Coord& a, const Coord& b) {
        return !(a==b);
  }

  /**
   * Returns the distance to const Coord& a
   */
  double distance(const Coord& a) const {
        return sqrt(sqrdist(a));
  }

  /**
   * Returns distance^2 to Coord a (omits square root).
   */
  double sqrdist(const Coord& a) const {
        double dx=x-a.x;
        double dy=y-a.y;
        return dx*dx + dy*dy;
  }

};

inline std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    return os << "(" << coord.x << "," << coord.y << ")";
}

#endif

