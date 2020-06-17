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

#include "inet/common/math/Domain.h"
#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"
#include "inet/common/Ptr.h"

namespace inet {

namespace math {

/**
 * This interface represents a mathematical function from domain D to range R.
 * R is a scalar type (double, simtime_t, quantity, etc.)
 * D is a Domain<>.
 */
template<typename R, typename D>
class INET_API IFunction : public cObject, public SharedBase<IFunction<R, D>>
{
  public:
    virtual ~IFunction() {}

    /**
     * Returns the valid range of the function as an interval.
     */
    virtual Interval<R> getRange() const = 0;

    /**
     * Returns the valid range of the function as an interval for the given domain.
     */
    virtual Interval<R> getRange(const typename D::I& i) const = 0;

    /**
     * Returns the valid domain of the function as an interval.
     */
    virtual typename D::I getDomain() const = 0;

    /**
     * Returns the value of the function at the given point. The returned value
     * falls into the range of the function. The provided point must fall into
     * the domain of the function.
     */
    virtual R getValue(const typename D::P& p) const = 0;

    /**
     * Subdivides the provided domain and calls back f with the subdomains and
     * the corresponding potentially simpler domain limited functions.
     */
    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const = 0;

    /**
     * Returns true if the function value is finite in the whole domain.
     */
    virtual bool isFinite() const = 0;

    /**
     * Returns true if the function value is finite in the given domain.
     */
    virtual bool isFinite(const typename D::I& i) const = 0;

    /**
     * Returns true if the function value is non-zero in the whole domain.
     */
    virtual bool isNonZero() const = 0;

    /**
     * Returns true if the function value is non-zero in the given domain.
     */
    virtual bool isNonZero(const typename D::I& i) const = 0;

    /**
     * Returns the minimum value for the whole domain.
     */
    virtual R getMin() const = 0;

    /**
     * Returns the minimum value for the given domain.
     */
    virtual R getMin(const typename D::I& i) const = 0;

    /**
     * Returns the maximum value for the whole domain.
     */
    virtual R getMax() const = 0;

    /**
     * Returns the maximum value for the given domain.
     */
    virtual R getMax(const typename D::I& i) const = 0;

    /**
     * Returns the mean value for the whole domain.
     */
    virtual R getMean() const = 0;

    /**
     * Returns the mean value for the given domain.
     */
    virtual R getMean(const typename D::I& i) const = 0;

    /**
     * Returns the integral value for the whole domain.
     */
    virtual R getIntegral() const = 0;

    /**
     * Returns the integral value for the given domain.
     */
    virtual R getIntegral(const typename D::I& i) const = 0;

    /**
     * Adds the provided function to this function.
     */
    virtual const Ptr<const IFunction<R, D>> add(const Ptr<const IFunction<R, D>>& o) const = 0;

    /**
     * Substracts the provided function from this function.
     */
    virtual const Ptr<const IFunction<R, D>> subtract(const Ptr<const IFunction<R, D>>& o) const = 0;

    /**
     * Multiplies the provided function with this function.
     */
    virtual const Ptr<const IFunction<R, D>> multiply(const Ptr<const IFunction<double, D>>& o) const = 0;

    /**
     * Divides this function with the provided function.
     */
    virtual const Ptr<const IFunction<double, D>> divide(const Ptr<const IFunction<R, D>>& o) const = 0;

    /**
     * Prints this function in human readable form to the provided stream for the whole domain.
     */
    virtual void print(std::ostream& os, int level = 0) const = 0;

    /**
     * Prints this function in a human readable form to the provided stream for the given domain.
     */
    virtual void print(std::ostream& os, const typename D::I& i, int level = 0) const = 0;

    /**
     * Prints the partitioning of this function in a human readable form to the provided stream for the given domain.
     */
    virtual void printPartitioning(std::ostream& os, const typename D::I& i, int level = 0) const = 0;

    /**
     * Prints a single partition of this function in a human readable form to the provided stream for the given domain.
     */
    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const = 0;

    /**
     * Prints the internal data structure of this function in a human readable form to the provided stream.
     */
    virtual void printStructure(std::ostream& os, int level = 0) const = 0;
};

template<typename R, typename ... T>
inline std::ostream& operator<<(std::ostream& os, const IFunction<R, Domain<T ...> >& f) {
    f.print(os);
    return os;
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_IFUNCTION_H_

