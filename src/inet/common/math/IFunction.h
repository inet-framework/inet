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
 * This class represents the domain of a mathematical function.
 */
template<typename ... T>
class INET_API Domain
{
  public:
    typedef Point<T ...> P;
    typedef Interval<T ...> I;
};

/**
 * This interface represents a mathematical function from domain D to range R.
 */
template<typename R, typename D>
class INET_API IFunction :
#if INET_PTR_IMPLEMENTATION == INET_STD_SHARED_PTR
    public std::enable_shared_from_this<Chunk>
#elif INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
    public IntrusivePtrCounter<IFunction<R, D>>
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
    virtual typename D::I getDomain() const = 0;

    /**
     * Returns the value of the function at the given point. The returned value
     * falls into the range of the function. The provided point must fall into
     * the domain of the function.
     */
    virtual R getValue(const typename D::P& p) const = 0;

    /**
     * Subdivides the provided domain and calls back the provided function with
     * the subdomains and the corresponding potentially simpler domain limited functions.
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
};

template<typename R, typename ... T>
inline std::ostream& operator<<(std::ostream& os, const IFunction<R, Domain<T ...> >& f)
{
    os << "f {" << std::endl;
    f.partition(f.getDomain(), [&] (const Interval<T ...>& i, const IFunction<R, Domain<T ...>> *g) {
        os << "  i " << i << " -> { ";
        iterateBoundaries<T ...>(i, std::function<void (const Point<T ...>&)>([&] (const Point<T ...>& p) {
            os << "@" << p << " = " << f.getValue(p) << ", ";
        }));
        os << "min = " << g->getMin(i) << ", max = " << g->getMax(i) << ", mean = " << g->getMean(i) << " }" << std::endl;
    });
    return os << "} min = " << f.getMin() << ", max = " << f.getMax() << ", mean = " << f.getMean();
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_IFUNCTION_H_

