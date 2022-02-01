//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINTERPOLATOR_H
#define __INET_IINTERPOLATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace math {

/**
 * This interface represents interpolation of values (y) between two points x1 and x2.
 * The types X and Y represent scalar values (double, simtime_t, or some quantity with unit).
 */
template<typename X, typename Y>
class INET_API IInterpolator : public cObject
{
  public:
    virtual ~IInterpolator() {}

    /**
     * Returns the interpolated value for the given x. The value of x must fall
     * into the closed interval [x1, x2].
     */
    virtual Y getValue(const X x1, const Y y1, const X x2, const Y y2, const X x) const = 0;

    /**
     * Returns the minimum interpolated value in the closed interval of [x1, x2].
     */
    virtual Y getMin(const X x1, const Y y1, const X x2, const Y y2) const = 0;

    /**
     * Returns the maximum interpolated value in the closed interval of [x1, x2].
     */
    virtual Y getMax(const X x1, const Y y1, const X x2, const Y y2) const = 0;

    /**
     * Returns the mean interpolated value in the closed interval of [x1, x2].
     */
    virtual Y getMean(const X x1, const Y y1, const X x2, const Y y2) const = 0;
};

} // namespace math

} // namespace inet

#endif

