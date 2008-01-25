/* -*- mode:c++ -*- ********************************************************
 * file:        FWMath.h
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

#ifndef FWMATH_H
#define FWMATH_H

//
// Support functions for mathematical operations
//

#include <math.h>


/* windows math.h doesn't define the PI variable so we have to do it
   by hand*/
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Constant for comparing doubles. Two doubles at most epsilon apart
   are declared equal.*/
#ifndef EPSILON
#define EPSILON 0.001
#endif


/**
 * @brief Support functions for mathematical operations.
 *
 * This class contains all kind of mathematical support functions
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @author Christian Frank
 */
class INET_API FWMath {

 public:

  /**
   * Returns the rest of a whole-numbered division.
   */
  static double mod(double dividend, double divisor) {
      double i;
      return modf(dividend/divisor, &i)*divisor;
  }

  /**
   * Returns the result of a whole-numbered division.
   */
  static double div(double dividend, double divisor) {
      double i;
      double f;
      f=modf(dividend/divisor, &i);
      return i;
  }

  /**
   * Tests whether two doubles are close enough to be declared equal.
   * @return true if parameters are at most epsilon apart, false
   * otherwise
   */
  static bool close(double one, double two) {
      return fabs(one-two)<EPSILON;
  }

  /**
   * @return 0 if i is close to 0, 1 if i is positive greater epsilon,
   * -1 if it is negative smaller epsilon.
   */
  static int stepfunction(double i) { return (i>EPSILON) ? 1 : close(i,0) ? 0 :-1; };


  /**
   * @return 1 if parameter greater or equal zero, -1 otherwise
   */
  static int sign(double i) { return (i>=0)? 1 : -1; };

  /**
   * @return integer that corresponds to rounded double parameter
   */
  static int round(double d) { return (int)(ceil(d-0.5)); }

  /**
   * @return greater of the given parameters
   */
  static double max(double a, double b) { return (a<b)? b : a; }

  /**
   * convert a dBm value into milli Watt
   */
  static double dBm2mW(double dBm){
      return pow(10.0, dBm/10.0);
  }

};

#endif
