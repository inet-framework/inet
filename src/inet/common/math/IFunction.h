//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFUNCTION_H
#define __INET_IFUNCTION_H

#include "inet/common/Ptr.h"
#include "inet/common/math/Domain.h"
#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"

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
    virtual void partition(const typename D::I& i, const std::function<void(const typename D::I&, const IFunction<R, D> *)> f) const = 0;

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

    virtual std::ostream& printOn(std::ostream& os) const override { print(os); return os; }
};

} // namespace math

} // namespace inet

#endif

