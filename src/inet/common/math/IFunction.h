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

#include "inet/common/math/IInterpolator.h"
#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"
#include "inet/common/Ptr.h"

namespace inet {

namespace math {

/**
 * This interface represents a mathematical function from domain DS to range R.
 */
template<typename R, typename ... DS>
class INET_API IFunction :
#if INET_PTR_IMPLEMENTATION == INET_STD_SHARED_PTR
    public std::enable_shared_from_this<Chunk>
#elif INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
    public IntrusivePtrCounter<IFunction<R, DS ...>>
#else
#error "Unknown Ptr implementation"
#endif
{
  public:
    virtual ~IFunction() {}

    /**
     * Returns the valid range of the function as a closed interval.
     */
    virtual Interval<R> getRange() const = 0;

    /**
     * Returns the valid domain of the function as a closed interval.
     */
    virtual Interval<DS ...> getDomain() const = 0;

    /**
     * Returns the value of the function at the given point. The returned value
     * falls into the range of the function. The provided point must fall into
     * the domain of the function.
     */
    virtual R getValue(const Point<DS ...>& p) const = 0;

    /**
     * Subdivides the provided domain and calls back the provided function with
     * the subdomains and the corresponding potentially simpler domain limited function.
     */
    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const = 0;

    /**
     * Returns a function that represents the same function as this limited to
     * the given domain.
     */
    virtual Ptr<const IFunction<R, DS ...>> limitDomain(const Interval<DS ...>& i) const = 0;

    /**
     * Returns the minimum value for the whole domain.
     */
    virtual R getMin() const = 0;

    /**
     * Returns the minimum value for the given domain.
     */
    virtual R getMin(const Interval<DS ...>& i) const = 0;

    /**
     * Returns the maximum value for the whole domain.
     */
    virtual R getMax() const = 0;

    /**
     * Returns the maximum value for the given domain.
     */
    virtual R getMax(const Interval<DS ...>& i) const = 0;

    /**
     * Returns the mean value for the whole domain.
     */
    virtual R getMean() const = 0;

    /**
     * Returns the mean value for the given domain.
     */
    virtual R getMean(const Interval<DS ...>& i) const = 0;

    /**
     * Adds the provided function to this function.
     */
    virtual const Ptr<const IFunction<R, DS ...>> add(const Ptr<const IFunction<R, DS ...>>& o) const = 0;

    /**
     * Substracts the provided function from this function.
     */
    virtual const Ptr<const IFunction<R, DS ...>> subtract(const Ptr<const IFunction<R, DS ...>>& o) const = 0;

    /**
     * Multiplies the provided function with this function.
     */
    virtual const Ptr<const IFunction<R, DS ...>> multiply(const Ptr<const IFunction<double, DS ...>>& o) const = 0;

    /**
     * Divides this function with the provided function.
     */
    virtual const Ptr<const IFunction<double, DS ...>> divide(const Ptr<const IFunction<R, DS ...>>& o) const = 0;
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_IFUNCTION_H_

