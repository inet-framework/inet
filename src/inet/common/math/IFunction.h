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

#ifndef __INET_MATH_IFUNCTION_H_
#define __INET_MATH_IFUNCTION_H_

#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

/**
 * This interface represents a  function from domain DS to range R.
 */
template<typename R, typename ... DS>
class INET_API IFunction
{
  public:
    virtual ~IFunction() {}

    virtual Interval<R> getRange() const = 0;
    virtual Interval<DS ...> getDomain() const = 0;

    virtual R getValue(const Point<DS ...>& p) const = 0;

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const = 0;
    virtual IFunction<R, DS ...> *limitDomain(const Interval<DS ...>& i) const = 0;

    virtual R getMin() const = 0;
    virtual R getMin(const Interval<DS ...>& i) const = 0;

    virtual R getMax() const = 0;
    virtual R getMax(const Interval<DS ...>& i) const = 0;

    virtual R getMean() const = 0;
    virtual R getMean(const Interval<DS ...>& i) const = 0;
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_IFUNCTION_H_

