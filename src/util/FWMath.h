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
#include "INETDefs.h"

/* Windows math.h doesn't define the the following variables: */
#ifndef M_E
#define M_E         2.7182818284590452354
#endif

#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074
#endif

#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765
#endif

#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif

#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962
#endif

#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154
#endif

#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif

#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440
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
      modf(dividend/divisor, &i);
      return i;
  }

  /**
   * Returns the remainder r on division of dividend a by divisor n,
   * using floored division. The remainder r has the same sign as the divisor n.
   */
  static double modulo(double a, double n) { return (a - n * floor(a/n)); }

  /**
   * Tests whether two doubles are close enough to be declared equal.
   * Returns true if parameters are at most epsilon apart, false
   * otherwise
   */
  static bool close(double one, double two) { return fabs(one-two)<EPSILON; }

  /**
   * Returns 0 if i is close to 0, 1 if i is positive and greater than epsilon,
   * or -1 if it is negative and less than epsilon.
   */
  static int stepfunction(double i) { return (i>EPSILON) ? 1 : close(i, 0) ? 0 :-1; };

  /**
   * Returns 1 if the parameter is greater or equal to zero, -1 otherwise
   */
  static int sign(double i) { return (i>=0)? 1 : -1; };

  /**
   * Returns an integer that corresponds to rounded double parameter
   */
  static int round(double d) { return (int)(ceil(d-0.5)); }

  /**
   * Discards the fractional part of the parameter, e.g. -3.8 becomes -3
   */
  static double floorToZero(double d) { return (d >= 0.0)? floor(d) : ceil(d); }

  /**
   * Returns the greater of the given parameters
   */
  static double max(double a, double b) { return (a<b)? b : a; }

  /**
   * Converts a dBm value into milliwatts
   */
  static double dBm2mW(double dBm) { return pow(10.0, dBm/10.0); }

  /**
   * Convert a mW value to dBm.
   */
  static double mW2dBm(double mW) { return (10 * log10(mW)); }
};

#endif
