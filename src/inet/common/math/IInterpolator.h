//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_MATH_IINTERPOLATOR_H_
#define __INET_MATH_IINTERPOLATOR_H_

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
    virtual ~IInterpolator() { }

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

#endif // #ifndef __INET_MATH_IINTERPOLATOR_H_

