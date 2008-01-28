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


#ifndef _COORD_H
#define _COORD_H

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
  Coord( const Coord& pos ) {
        x=pos.x;
        y=pos.y;
  }

  /** Initializes coordinates.*/
  Coord( const Coord* pos ) {
        x=pos->x;
        y=pos->y;
  }

  std::string info() const {
        std::stringstream os;
        os << "(" << x << "," << y << ")";
        return os.str();
  }

  /** Adds two coordinate vectors.*/
  friend Coord operator+(Coord a, Coord b) {
        return Coord(a.x+b.x, a.y+b.y);
  }

  /** Subtracts two coordinate vectors.*/
  friend Coord operator-(Coord a, Coord b) {
        return Coord(a.x-b.x, a.y-b.y);
  }

  /** Multiplies a coordinate vector by a real number.*/
  friend Coord operator*(Coord a, double f) {
        return Coord(a.x*f, a.y*f);
  }

  /** Divides a coordinate vector by a real number.*/
  friend Coord operator/(Coord a, double f) {
        return Coord(a.x/f, a.y/f);
  }

  /** Adds coordinate vector b to a.*/
  Coord operator+=(Coord a) {
    x+=a.x;
    y+=a.y;
    return *this;
  }

  /** Assigns a this.*/
  Coord operator=(Coord a) {
    x=a.x;
    y=a.y;
    return *this;
  }

  /** Subtracts coordinate vector b from a.*/
  Coord operator-=(Coord a) {
    x-=a.x;
    y-=a.y;
    return *this;
  }

  /**
   * Tests whether two coordinate vectors are equal. Because
   * coordinates are of type double, this is done through the
   * FWMath::close function.
   */
  friend bool operator==(Coord a, Coord b) {
        return FWMath::close(a.x,b.x) && FWMath::close(a.y,b.y);
  }

  /**
   * Tests whether two coordinate vectors are not equal. Negation of
   * the operator==.
   */
  friend bool operator!=(Coord a, Coord b) {
        return !(a==b);
  }

  /**
   * Returns the distance to Coord a
   */
  double distance( const Coord a ) const {
    Coord dist=*this-a;
    return sqrt( dist.x*dist.x + dist.y*dist.y);
  }

  /**
   * Returns distance^2 to Coord a (omits square root).
   */
  double sqrdist( const Coord a ) const {
    Coord dist=*this-a;
    return dist.x*dist.x + dist.y*dist.y;
  }

};

inline std::ostream& operator<<(std::ostream& os, const Coord& coord)
{
    return os << "(" << coord.x << "," << coord.y << ")";
}

#endif

