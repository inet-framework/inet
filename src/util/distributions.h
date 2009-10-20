//-------------------------------------------------------------------------------
//	distributions.h --
//
//	This file declares classes for additional probability distributions
//	which are not implemented in OMNeT++.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#ifndef __DISTRIBUTIONS_H
#define __DISTRIBUTIONS_H


#include <omnetpp.h>


/**
 * Truncated lognormal distribution at both ends.
 * It is implemented with a loop that discards a value outside the range
 * until it comes within a range. This means that the execution time
 * is not bounded.
 *
 * The "scale" and "shape" parameters serve as parameters to the lognormal
 * distribution <i>before</i> truncation.
 *
 * @param m  "scale" parameter, m>0
 * @param w  "shape" parameter, w>0
 * @param t   truncation mode, 'l'->left, 'r'->right, 'b'->both
 * @param min left truncation point
 * @param max right truncation point
 * @param rng the underlying random number generator
 */
double trunc_lognormal(double m, double w, char t, double min, double max, int rng=0);

///**
// * SimTime version of trunclognormal(double,double,double,double,int), for convenience.
// */
//inline SimTime trunclognormal(SimTime m, SimTime w, SimTime min, SimTime max,
//		int rng = 0) {
//	return trunclognormal(m.dbl(), w.dbl(), min.dbl(), max.dbl(), rng);
//}

/**
 * Truncated shifted generalized Pareto distribution at both ends.
 * It is implemented with a loop that discards a value outside the range
 * until it comes within a range. This means that the execution time
 * is not bounded.
 *
 * The following parameters (except for "min" and "max") serve as parameters
 * to the shifted generalized Pareto distribution <i>before</i> truncation.
 *
 * @param a,b  the usual parameters for generalized Pareto
 * @param c    shift parameter for left-shift
 * @param t   truncation mode, 'l'->left, 'r'->right, 'b'->both
 * @param min left truncation point
 * @param max right truncation point
 * @param rng the underlying random number generator
 */
double trunc_pareto_shifted(double a, double b, double c, char t, double min, double max, int rng=0);


#endif  // __DISTRIBUTIONS_H

