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
#include <cnedfunction.h>
#include <unitconversion.h>


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
 * @param min left truncation point
 * @param max right truncation point
 * @param t   truncation mode, true->both (default), false->left only
 * @param rng the underlying random number generator
 */
double trunc_lognormal(double m, double w, double min, double max, bool t=true, int rng=0);

///**
// * SimTime version of trunclognormal(double,double,double,double,int), for convenience.
// */
//inline SimTime trunclognormal(SimTime m, SimTime w, SimTime min, SimTime max,
//		int rng = 0) {
//	return trunclognormal(m.dbl(), w.dbl(), min.dbl(), max.dbl(), rng);
//}

/**
 * Truncated Pareto distribution at both ends.
 * It is implemented with a loop that discards a value outside the range
 * until it comes within a range. This means that the execution time
 * is not bounded.
 *
 * The following parameters (except for "max") serve as parameters
 * to the Pareto distribution <i>before</i> truncation.
 *
 * @param k     scale parameter (> 0, minimum value)
 * @param alpha shape parameter (> 0)
 * @param m     right truncation point (maximum value)
 * @param rng   the underlying random number generator
 */
double trunc_pareto(double k, double alpha, double m, int rng=0);

///**
// * Truncated shifted generalized Pareto distribution at both ends.
// * It is implemented with a loop that discards a value outside the range
// * until it comes within a range. This means that the execution time
// * is not bounded.
// *
// * The following parameters (except for "min" and "max") serve as parameters
// * to the shifted generalized Pareto distribution <i>before</i> truncation.
// *
// * @param a,b  the usual parameters for generalized Pareto
// * @param c    shift parameter for left-shift
// * @param min left truncation point
// * @param max right truncation point
// * @param t   truncation mode, true->both (default), false->left only
// * @param rng the underlying random number generator
// */
//double trunc_pareto_shifted(double a, double b, double c, double min, double max, bool t=true, int rng=0);


#endif  // __DISTRIBUTIONS_H

