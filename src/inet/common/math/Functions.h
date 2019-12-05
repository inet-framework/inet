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

#ifndef __INET_MATH_FUNCTIONS_H_
#define __INET_MATH_FUNCTIONS_H_

#include <algorithm>
#include "inet/common/math/IFunction.h"
#include "inet/common/math/Interpolators.h"

namespace inet {

namespace math {

using namespace inet::units::values;

/**
 * Verifies that partitioning on a domain is correct in the sense that the
 * original function and the function over the partition return the same
 * values at the corners and center of the subdomain.
 */
template<typename R, typename D>
class INET_API FunctionChecker
{
  protected:
    const Ptr<const IFunction<R, D>> function;

  public:
    FunctionChecker(const Ptr<const IFunction<R, D>>& function) : function(function) { }

    void check() const {
        check(function->getDomain());
    }

    void check(const typename D::I& i) const {
        function->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            auto check = std::function<void (const typename D::P&)>([&] (const typename D::P& p) {
                if (i1.contains(p)) {
                    R r = function->getValue(p);
                    R r1 = f1->getValue(p);
                    ASSERT(r == r1 || (std::isnan(toDouble(r)) && std::isnan(toDouble(r1))));
                }
            });
            iterateCorners(i1, check);
            check((i1.getLower() + i1.getUpper()) / 2);
        });
    }
};

template<typename R, typename D>
class INET_API ConstantFunction;

template<typename R, typename D>
class INET_API DomainLimitedFunction;

template<typename R, typename D>
class INET_API AdditionFunction;

template<typename R, typename D>
class INET_API SubtractionFunction;

template<typename R, typename D>
class INET_API MultiplicationFunction;

template<typename R, typename D>
class INET_API DivisionFunction;

template<typename R, typename D, int DIMS, typename RI, typename DI>
class INET_API IntegratedFunction;

template<typename R, typename D, int DIMENSION, typename X>
class INET_API ApproximatedFunction;

/**
 * Useful base class for most IFunction implementations with some default behavior.
 */
template<typename R, typename D>
class INET_API FunctionBase : public IFunction<R, D>
{
  public:
    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        auto m = (1 << std::tuple_size<typename D::P::type>::value) - 1;
        if (i.getFixed() == m) {
            ASSERT(i.getLower() == i.getUpper());
            ConstantFunction<R, D> g(this->getValue(i.getLower()));
            callback(i, &g);
        }
        else
            throw cRuntimeError("Cannot partition %s, interval = %s", this->getClassName(), i.str().c_str());
    }

    virtual Interval<R> getRange() const override {
        return getRange(getDomain());
    }

    virtual Interval<R> getRange(const typename D::I& i) const override {
        return Interval<R>(getLowerBound<R>(), getUpperBound<R>(), 0b1, 0b1, 0b0);
    }

    virtual typename D::I getDomain() const override {
        return typename D::I(D::P::getLowerBounds(), D::P::getUpperBounds(), 0b0, 0b0, 0b0);
    }

    virtual bool isFinite() const override { return isFinite(getDomain()); }
    virtual bool isFinite(const typename D::I& i) const override {
        bool result = true;
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result &= f1->isFinite(i1);
        });
        return result;
    }

    virtual R getMin() const override { return getMin(getDomain()); }
    virtual R getMin(const typename D::I& i) const override {
        R result(getUpperBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result = std::min(f1->getMin(i1), result);
        });
        return result;
    }

    virtual R getMax() const override { return getMax(getDomain()); }
    virtual R getMax(const typename D::I& i) const override {
        R result(getLowerBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result = std::max(f1->getMax(i1), result);
        });
        return result;
    }

    virtual R getMean() const override { return getMean(getDomain()); }
    virtual R getMean(const typename D::I& i) const override {
        return getIntegral(i) / i.getVolume();
    }

    virtual R getIntegral() const override { return getIntegral(getDomain()); }
    virtual R getIntegral(const typename D::I& i) const override {
        R result(0);
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            double volume = i1.getVolume();
            R value = f1->getMean(i1);
            if (!(value == R(0) && std::isinf(volume)))
                result += volume * value;
        });
        return result;
    }

    virtual const Ptr<const IFunction<R, D>> add(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<AdditionFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, D>> subtract(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<SubtractionFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, D>> multiply(const Ptr<const IFunction<double, D>>& o) const override {
        return makeShared<MultiplicationFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<double, D>> divide(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<DivisionFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual void print(std::ostream& os, int level = 0) const override {
        print(os, getDomain(), level);
    }

    virtual void print(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << std::string(level, ' ') << "function" << D() << " → ";
        printUnit(os, R());
        os << std::string(level, ' ') << " {\n  domain = " << i << " → range = " << getRange() << "\n";
        os << std::string(level, ' ') << "  structure =\n    ";
        printStructure(os, level + 4);
        os << std::string(level, ' ') << "\n";
        os << std::string(level, ' ') << "  partitioning = {\n";
        printPartitioning(os, i, level + 4);
        os << std::string(level, ' ') << "  } min = " << getMin(i) << ", max = " << getMax(i) << ", mean = " << getMean(i) << "\n}\n";
    }

    virtual void printPartitioning(std::ostream& os, const typename D::I& i, int level) const override {
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            os << std::string(level, ' ');
            f1->printPartition(os, i1, level);
        });
    }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "over " << i << " → {";
        iterateCorners(i, std::function<void (const typename D::P&)>([&] (const typename D::P& p) {
            os << "\n" << std::string(level + 2, ' ') << "at " << p << " → " << this->getValue(p);
        }));
        os << "\n" << std::string(level, ' ') << "} min = " << getMin(i) << ", max = " << getMax(i) << ", mean = " << getMean(i) << "\n";
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        auto className = this->getClassName();
        if (!strncmp(className, "inet::math::", 12))
            className += 12;
        else if (!strncmp(className, "inet::", 6))
            className += 6;
        os << className;
    }
};

template<typename R, typename D, int DIMS, typename RI, typename DI>
Ptr<const IFunction<RI, DI>> integrate(const Ptr<const IFunction<R, D>>& f) {
    return makeShared<IntegratedFunction<R, D, DIMS, RI, DI>>(f);
}

template<typename R, typename D>
class INET_API DomainLimitedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function;
    const Interval<R> range;
    const typename D::I domain;

  public:
    DomainLimitedFunction(const Ptr<const IFunction<R, D>>& function, const typename D::I& domain) :
        function(function), range(Interval<R>(function->getMin(domain), function->getMax(domain), 0b1, 0b1, 0b0)), domain(domain)
    { }

    virtual Interval<R> getRange() const override { return range; }
    virtual typename D::I getDomain() const override { return domain; }

    virtual R getValue(const typename D::P& p) const override {
        ASSERT(domain.contains(p));
        return function->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        const auto& i1 = i.getIntersected(domain);
        if (!i1.isEmpty())
            function->partition(i1, callback);
    }

    virtual R getMin(const typename D::I& i) const override {
        return function->getMin(i.getIntersected(domain));
    }

    virtual R getMax(const typename D::I& i) const override {
        return function->getMax(i.getIntersected(domain));
    }

    virtual R getMean(const typename D::I& i) const override {
        return function->getMean(i.getIntersected(domain));
    }

    virtual R getIntegral(const typename D::I& i) const override {
        return function->getIntegral(i.getIntersected(domain));
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(DomainLimited, domain = " << domain << "\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename D>
Ptr<const DomainLimitedFunction<R, D>> makeFirstQuadrantLimitedFunction(const Ptr<const IFunction<R, D>>& f) {
    auto m = (1 << std::tuple_size<typename D::P::type>::value) - 1;
    typename D::I i(D::P::getZero(), D::P::getUpperBounds(), m, 0, 0);
    return makeShared<DomainLimitedFunction<R, D>>(f, i);
}

template<typename R, typename D>
class INET_API ConstantFunction : public FunctionBase<R, D>
{
  protected:
    const R value;

  public:
    ConstantFunction(R value) : value(value) { }

    virtual R getConstantValue() const { return value; }

    virtual Interval<R> getRange() const override { return Interval<R>(value, value, 0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override { return value; }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override { return std::isfinite(toDouble(value)); }
    virtual R getMin(const typename D::I& i) const override { return value; }
    virtual R getMax(const typename D::I& i) const override { return value; }
    virtual R getMean(const typename D::I& i) const override { return value; }
    virtual R getIntegral(const typename D::I& i) const override { return value == R(0) ? value : value * i.getVolume(); }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "constant over " << i << "\n" << std::string(level + 2, ' ') << "→ " << value << std::endl;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "Constant " << value;
    }
};

/**
 * Some constant value r between lower and upper and zero otherwise.
 */
template<typename R, typename X>
class INET_API OneDimensionalBoxcarFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X lower;
    const X upper;
    const R value;

  public:
    OneDimensionalBoxcarFunction(X lower, X upper, R value) : lower(lower), upper(upper), value(value) {
        ASSERT(value > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), value, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X>& p) const override {
        return std::get<0>(p) < lower || std::get<0>(p) >= upper ? R(0) : value;
    }

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, Domain<X>> *)> callback) const override {
        const auto& i1 = i.getIntersected(Interval<X>(getLowerBound<X>(), Point<X>(lower), 0b0, 0b0, 0b0));
        if (!i1.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            callback(i1, &g);
        }
        const auto& i2 = i.getIntersected(Interval<X>(Point<X>(lower), Point<X>(upper), 0b1, 0b0, 0b0));
        if (!i2.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(value);
            callback(i2, &g);
        }
        const auto& i3 = i.getIntersected(Interval<X>(Point<X>(upper), getUpperBound<X>(), 0b1, 0b0, 0b0));
        if (!i3.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            callback(i3, &g);
        }
    }

    virtual bool isFinite(const Interval<X>& i) const override { return std::isfinite(toDouble(value)); }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(OneDimensionalBoxcar, [" << lower << " … " << upper << "] → " << value << ")";
    }
};

/**
 * Some constant value r between (lowerX, lowerY) and (upperX, upperY) and zero otherwise.
 */
template<typename R, typename X, typename Y>
class INET_API TwoDimensionalBoxcarFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const X lowerX;
    const X upperX;
    const Y lowerY;
    const Y upperY;
    const R value;

  protected:
    void call(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback, R r) const {
        if (!i.isEmpty()) {
            ConstantFunction<R, Domain<X, Y>> g(r);
            callback(i, &g);
        }
    }

  public:
    TwoDimensionalBoxcarFunction(X lowerX, X upperX, Y lowerY, Y upperY, R value) :
        lowerX(lowerX), upperX(upperX), lowerY(lowerY), upperY(upperY), value(value)
    {
        ASSERT(value > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), value, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X, Y>& p) const override {
        return std::get<0>(p) < lowerX || std::get<0>(p) >= upperX || std::get<1>(p) < lowerY || std::get<1>(p) >= upperY ? R(0) : value;
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), getLowerBound<Y>()), Point<X, Y>(X(lowerX), Y(lowerY)), 0b00, 0b00, 0b00)), callback, R(0));
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(lowerX), getLowerBound<Y>()), Point<X, Y>(X(upperX), Y(lowerY)), 0b10, 0b00, 0b00)), callback, R(0));
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(upperX), getLowerBound<Y>()), Point<X, Y>(getUpperBound<X>(), Y(lowerY)), 0b10, 0b00, 0b00)), callback, R(0));

        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(lowerY)), Point<X, Y>(X(lowerX), Y(upperY)), 0b01, 0b00, 0b00)), callback, R(0));
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(lowerY)), Point<X, Y>(X(upperX), Y(upperY)), 0b11, 0b00, 0b00)), callback, value);
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(upperX), Y(lowerY)), Point<X, Y>(getUpperBound<X>(), Y(upperY)), 0b11, 0b00, 0b00)), callback, R(0));

        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(upperY)), Point<X, Y>(X(lowerX), getUpperBound<Y>()), 0b01, 0b00, 0b00)), callback, R(0));
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(upperY)), Point<X, Y>(X(upperX), getUpperBound<Y>()), 0b11, 0b00, 0b00)), callback, R(0));
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(upperX), Y(upperY)), Point<X, Y>(getUpperBound<X>(), getUpperBound<Y>()), 0b11, 0b00, 0b00)), callback, R(0));
    }

    virtual bool isFinite(const Interval<X, Y>& i) const override { return std::isfinite(toDouble(value)); }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(TwoDimensionalBoxcar, [" << lowerX << " … " << upperX << "] x [" << lowerY << " … " << upperY << "] → " << value << ")";
    }
};

/**
 * Linear in one dimension and constant in the others.
 */
template<typename R, typename D>
class INET_API UnilinearFunction : public FunctionBase<R, D>
{
  protected:
    const typename D::P lower; // value is ignored in all but one dimension
    const typename D::P upper; // value is ignored in all but one dimension
    const R rLower;
    const R rUpper;
    const int dimension;

  public:
    UnilinearFunction(typename D::P lower, typename D::P upper, R rLower, R rUpper, int dimension) :
        lower(lower), upper(upper), rLower(rLower), rUpper(rUpper), dimension(dimension) { }

    virtual const typename D::P& getLower() const { return lower; }
    virtual const typename D::P& getUpper() const { return upper; }
    virtual R getRLower() const { return rLower; }
    virtual R getRUpper() const { return rUpper; }
    virtual int getDimension() const { return dimension; }

    virtual double getA() const { return toDouble(rUpper - rLower) / toDouble(upper.get(dimension) - lower.get(dimension)); }
    virtual double getB() const { return (toDouble(rLower) * upper.get(dimension) - toDouble(rUpper) * lower.get(dimension)) / (upper.get(dimension) - lower.get(dimension)); }

    virtual Interval<R> getRange() const override { return Interval<R>(std::min(rLower, rUpper), std::max(rLower, rUpper), 0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override {
        double alpha = (p - lower).get(dimension) / (upper - lower).get(dimension);
        return rLower * (1 - alpha) + rUpper * alpha;
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return std::isfinite(toDouble(rLower)) && std::isfinite(toDouble(rUpper));
    }

    virtual R getMin(const typename D::I& i) const override {
        return std::min(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const typename D::I& i) const override {
        return std::max(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMean(const typename D::I& i) const override {
        return getValue((i.getLower() + i.getUpper()) / 2);
    }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "linear in dim. " << dimension << " over " << i << "\n"
           << std::string(level + 2, ' ') << "→ " << getValue(i.getLower()) << " … " << getValue(i.getUpper()) << std::endl;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "Linear, " << lower << " … " << upper << " → " << rLower << " … " << rUpper;
    }
};

/**
 * Linear in two dimensions and constant in the others.
 */
template<typename R, typename D>
class INET_API BilinearFunction : public FunctionBase<R, D>
{
  protected:
    const typename D::P lowerLower; // value is ignored in all but two dimensions
    const typename D::P lowerUpper; // value is ignored in all but two dimensions
    const typename D::P upperLower; // value is ignored in all but two dimensions
    const typename D::P upperUpper; // value is ignored in all but two dimensions
    const R rLowerLower;
    const R rLowerUpper;
    const R rUpperLower;
    const R rUpperUpper;
    const int dimension1;
    const int dimension2;

  protected:
    typename D::I getOtherInterval(const typename D::I& i) const {
        const typename D::P& lower = i.getLower();
        const typename D::P& upper = i.getUpper();
        typename D::P lowerUpper = D::P::getZero();
        typename D::P upperLower = D::P::getZero();
        lowerUpper.set(dimension1, lower.get(dimension1));
        lowerUpper.set(dimension2, upper.get(dimension2));
        upperLower.set(dimension1, upper.get(dimension1));
        upperLower.set(dimension2, lower.get(dimension2));
        return typename D::I(lowerUpper, upperLower, i.getLowerClosed(), i.getUpperClosed(), i.getFixed());
    }

  public:
    BilinearFunction(const typename D::P& lowerLower, const typename D::P& lowerUpper, const typename D::P& upperLower, const typename D::P& upperUpper,
                     const R rLowerLower, const R rLowerUpper, const R rUpperLower, const R rUpperUpper, const int dimension1, const int dimension2) :
        lowerLower(lowerLower), lowerUpper(lowerUpper), upperLower(upperLower), upperUpper(upperUpper),
        rLowerLower(rLowerLower), rLowerUpper(rLowerUpper), rUpperLower(rUpperLower), rUpperUpper(rUpperUpper),
        dimension1(dimension1), dimension2(dimension2) { }

    virtual const typename D::P& getLowerLower() const { return lowerLower; }
    virtual const typename D::P& getLowerUpper() const { return lowerUpper; }
    virtual const typename D::P& getUpperLower() const { return upperLower; }
    virtual const typename D::P& getUpperUpper() const { return upperUpper; }
    virtual R getRLowerLower() const { return rLowerLower; }
    virtual R getRLowerUpper() const { return rLowerUpper; }
    virtual R getRUpperLower() const { return rUpperLower; }
    virtual R getRUpperUpper() const { return rUpperUpper; }
    virtual int getDimension1() const { return dimension1; }
    virtual int getDimension2() const { return dimension2; }

    virtual Interval<R> getRange() const override { return Interval<R>(std::min(std::min(rLowerLower, rLowerUpper), std::min(rUpperLower, rUpperUpper)),
                                                                       std::max(std::max(rLowerLower, rLowerUpper), std::max(rUpperLower, rUpperUpper)),
                                                                       0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override {
        double lowerAlpha = (p - lowerLower).get(dimension1) / (upperLower - lowerLower).get(dimension1);
        R rLower = rLowerLower * (1 - lowerAlpha) + rUpperLower * lowerAlpha;
        const typename D::P lower = lowerLower * (1 - lowerAlpha) + upperLower * lowerAlpha;

        double upperAlpha = (p - lowerUpper).get(dimension1) / (upperUpper - lowerUpper).get(dimension1);
        R rUpper = rLowerUpper * (1 - upperAlpha) + rUpperUpper * upperAlpha;
        const typename D::P upper = lowerUpper * (1 - upperAlpha) + upperUpper * upperAlpha;

        double alpha = (p - lower).get(dimension2) / (upper - lower).get(dimension2);
        return rLower * (1 - alpha) + rUpper * alpha;
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual R getMin(const typename D::I& i) const override {
        auto j = getOtherInterval(i);
        return std::min(std::min(getValue(i.getLower()), getValue(j.getLower())), std::min(getValue(j.getUpper()), getValue(i.getUpper())));
    }

    virtual R getMax(const typename D::I& i) const override {
        auto j = getOtherInterval(i);
        return std::max(std::max(getValue(i.getLower()), getValue(j.getLower())), std::max(getValue(j.getUpper()), getValue(i.getUpper())));
    }

    virtual R getMean(const typename D::I& i) const override {
        return getValue((i.getLower() + i.getUpper()) / 2);
    }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "bilinear in dims. " << dimension1 << ", " << dimension2 << " over " << i << "\n"
           << std::string(level + 2, ' ') << "→ " << getValue(i.getLower()) << " … " << getValue(i.getUpper()) << std::endl;
    }
};

/**
 * Interpolated (e.g. constant, linear) between intervals defined by points on the X axis.
 */
template<typename R, typename X>
class INET_API OneDimensionalInterpolatedFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const std::map<X, std::pair<R, const IInterpolator<X, R> *>> rs;

  public:
    OneDimensionalInterpolatedFunction(const std::map<X, R>& rs, const IInterpolator<X, R> *interpolator) : rs([&] () {
        std::map<X, std::pair<R, const IInterpolator<X, R> *>> result;
        for (auto it : rs)
            result[it.first] = {it.second, interpolator};
        return result;
    } ()) { }

    OneDimensionalInterpolatedFunction(const std::map<X, std::pair<R, const IInterpolator<X, R> *>>& rs) : rs(rs) { }

    virtual R getValue(const Point<X>& p) const override {
        X x = std::get<0>(p);
        auto it = rs.equal_range(x);
        auto& lt = it.first;
        auto& ut = it.second;
        // TODO: this nested if looks horrible
        if (lt != rs.end() && lt->first == x) {
            if (ut == rs.end())
                return lt->second.first;
            else {
                const auto interpolator = lt->second.second;
                return interpolator->getValue(lt->first, lt->second.first, ut->first, ut->second.first, x);
            }
        }
        else {
            ASSERT(lt != rs.end() && ut != rs.end());
            lt--;
            const auto interpolator = lt->second.second;
            return interpolator->getValue(lt->first, lt->second.first, ut->first, ut->second.first, x);
        }
    }

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, Domain<X>> *)> callback) const override {
        // loop from less or equal than lower to greater or equal than upper inclusive both ends
        auto lt = rs.lower_bound(std::get<0>(i.getLower()));
        auto ut = rs.upper_bound(std::get<0>(i.getUpper()));
        if (lt->first > std::get<0>(i.getLower()))
            lt--;
        if (ut == rs.end())
            ut--;
        for (auto it = lt; it != ut; it++) {
            auto jt = it;
            jt++;
            auto kt = jt;
            kt++;
            auto i1 = i.getIntersected(Interval<X>(Point<X>(it->first), Point<X>(jt->first), 0b1, kt == rs.end() ? 0b1 : 0b0, 0b0));
            if (!i1.isEmpty()) {
                const auto interpolator = it->second.second;
                auto xLower = std::get<0>(i1.getLower());
                auto xUpper = std::get<0>(i1.getUpper());
                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator)) {
                    auto value = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, (xLower + xUpper) / 2);
                    ConstantFunction<R, Domain<X>> g(value);
                    callback(i1, &g);
                }
                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                    auto yLower = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, xLower);
                    auto yUpper = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, xUpper);
                    UnilinearFunction<R, Domain<X>> g(xLower, xUpper, yLower, yUpper, 0);
                    simplifyAndCall(i1, &g, callback);
                }
                else
                    throw cRuntimeError("TODO");
            }
        }
    }

    virtual bool isFinite(const Interval<X>& i) const override { return true; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(OneDimensionalInterpolated";
        auto size = rs.size();
        for (auto entry : rs) {
            const char *interpolatorClassName = nullptr;
            if (entry.second.second != nullptr) {
                interpolatorClassName = entry.second.second->getClassName();
                if (!strncmp(interpolatorClassName, "inet::math::", 12))
                    interpolatorClassName += 12;
                else if (!strncmp(interpolatorClassName, "inet::", 6))
                    interpolatorClassName += 6;
            }
            if (size < 8) {
                os << ", " << entry.first << " → " << entry.second.first;
                if (interpolatorClassName != nullptr)
                    os << ", " << interpolatorClassName;
            }
            else {
                os << "\n" << std::string(level + 2, ' ') << entry.first << " → " << entry.second.first;
                if (interpolatorClassName != nullptr)
                    os << "\n" << std::string(level + 4, ' ') << interpolatorClassName;
            }
        }
        os << ")";
    }
};

//template<typename R, typename X, typename Y>
//class INET_API TwoDimensionalInterpolatedFunction : public Function<R, X, Y>
//{
//  protected:
//    const IInterpolator<T, R>& interpolatorX;
//    const IInterpolator<T, R>& interpolatorY;
//    const std::vector<std::tuple<X, Y, R>> rs;
//
//  public:
//    TwoDimensionalInterpolatedFunction(const IInterpolator<T, R>& interpolatorX, const IInterpolator<T, R>& interpolatorY, const std::vector<std::tuple<X, Y, R>>& rs) :
//        interpolatorX(interpolatorX), interpolatorY(interpolatorY), rs(rs) { }
//
//    virtual R getValue(const Point<T>& p) const override {
//        throw cRuntimeError("TODO");
//    }
//
//    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
//        throw cRuntimeError("TODO");
//    }
//};

//template<typename R, typename D0, typename D>
//class INET_API FunctionInterpolatingFunction : public Function<R, D>
//{
//  protected:
//    const IInterpolator<R, D0>& interpolator;
//    const std::map<D0, const IFunction<R, D> *> fs;
//
//  public:
//    FunctionInterpolatingFunction(const IInterpolator<R, D0>& interpolator, const std::map<D0, const IFunction<R, D> *>& fs) : interpolator(interpolator), fs(fs) { }
//
//    virtual R getValue(const Point<D0, D>& p) const override {
//        D0 x = std::get<0>(p);
//        auto lt = fs.lower_bound(x);
//        auto ut = fs.upper_bound(x);
//        ASSERT(lt != fs.end() && ut != fs.end());
//        typename D::P q;
//        return i.get(lt->first, lt->second->getValue(q), ut->first, ut->second->getValue(q), x);
//    }
//
//    virtual void partition(const Interval<D0, D>& i, const std::function<void (const Interval<D0, D>&, const IFunction<R, D> *)> callback) const override {
//        throw cRuntimeError("TODO");
//    }
//};

template<typename R, typename X>
class INET_API GaussFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X mean;
    const X stddev;

  public:
    GaussFunction(X mean, X stddev) : mean(mean), stddev(stddev) { }

    virtual R getValue(const Point<X>& p) const override {
        static const double c = 1 / sqrt(2 * M_PI);
        X x = std::get<0>(p);
        double a = toDouble((x - mean) / stddev);
        return R(c / toDouble(stddev) * std::exp(-0.5 * a * a));
    }
};

/**
 * Combines 2 one-dimensional functions into a two-dimensional function.
 */
template<typename R, typename X, typename Y>
class INET_API OrthogonalCombinatorFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<X>>> functionX;
    const Ptr<const IFunction<double, Domain<Y>>> functionY;

  public:
    OrthogonalCombinatorFunction(const Ptr<const IFunction<R, Domain<X>>>& functionX, const Ptr<const IFunction<double, Domain<Y>>>& functionY) :
        functionX(functionX), functionY(functionY) { }

    virtual Interval<X, Y> getDomain() const override {
        const auto& fDomain = functionX->getDomain();
        const auto& gDomain = functionY->getDomain();
        Point<X, Y> lower(std::get<0>(fDomain.getLower()), std::get<0>(gDomain.getLower()));
        Point<X, Y> upper(std::get<0>(fDomain.getUpper()), std::get<0>(gDomain.getUpper()));
        auto lowerClosed = fDomain.getLowerClosed() << 1 | gDomain.getLowerClosed();
        auto upperClosed = fDomain.getUpperClosed() << 1 | gDomain.getUpperClosed();
        auto fixed = fDomain.getFixed() << 1 | gDomain.getFixed();
        return Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed);
    }

    virtual R getValue(const Point<X, Y>& p) const override {
        return functionX->getValue(std::get<0>(p)) * functionY->getValue(std::get<1>(p));
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
        Interval<X> ix(Point<X>(std::get<0>(i.getLower())), Point<X>(std::get<0>(i.getUpper())), (i.getLowerClosed() & 0b10) >> 1, (i.getUpperClosed() & 0b10) >> 1, (i.getFixed() & 0b10) >> 1);
        Interval<Y> iy(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), (i.getLowerClosed() & 0b01) >> 0, (i.getUpperClosed() & 0b01) >> 0, (i.getFixed() & 0b01) >> 0);
        functionX->partition(ix, [&] (const Interval<X>& ix1, const IFunction<R, Domain<X>> *fx1) {
            functionY->partition(iy, [&] (const Interval<Y>& iy1, const IFunction<double, Domain<Y>> *fy1) {
                Point<X, Y> lower(std::get<0>(ix1.getLower()), std::get<0>(iy1.getLower()));
                Point<X, Y> upper(std::get<0>(ix1.getUpper()), std::get<0>(iy1.getUpper()));
                auto lowerClosed = (ix1.getLowerClosed() << 1) | (iy1.getLowerClosed() << 0);
                auto upperClosed = (ix1.getUpperClosed() << 1) | (iy1.getUpperClosed() << 0);
                auto fixed = (ix1.getFixed() << 1) | (iy1.getFixed() << 0);
                if (auto fx1c = dynamic_cast<const ConstantFunction<R, Domain<X>> *>(fx1)) {
                    if (auto fy1c = dynamic_cast<const ConstantFunction<double, Domain<Y>> *>(fy1)) {
                        ConstantFunction<R, Domain<X, Y>> g(fx1c->getConstantValue() * fy1c->getConstantValue());
                        callback(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g);
                    }
                    else if (auto fy1l = dynamic_cast<const UnilinearFunction<double, Domain<Y>> *>(fy1)) {
                        UnilinearFunction<R, Domain<X, Y>> g(lower, upper, fy1l->getValue(iy1.getLower()) * fx1c->getConstantValue(), fy1l->getValue(iy1.getUpper()) * fx1c->getConstantValue(), 1);
                        simplifyAndCall(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto fx1l = dynamic_cast<const UnilinearFunction<R, Domain<X>> *>(fx1)) {
                    if (auto fy1c = dynamic_cast<const ConstantFunction<double, Domain<Y>> *>(fy1)) {
                        UnilinearFunction<R, Domain<X, Y>> g(lower, upper, fx1l->getValue(ix1.getLower()) * fy1c->getConstantValue(), fx1l->getValue(ix1.getUpper()) * fy1c->getConstantValue(), 0);
                        simplifyAndCall(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g, callback);
                    }
                    else {
                        // QuadraticFunction<double, Domain<X, Y>> g();
                        throw cRuntimeError("TODO");
                    }
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(# ";
        functionX->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        functionY->printStructure(os, level + 3);
        os << ")";
    }
};

/**
 * Shifts the domain of a function.
 */
template<typename R, typename D>
class INET_API ShiftFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function;
    const typename D::P shift;

  public:
    ShiftFunction(const Ptr<const IFunction<R, D>>& f, const typename D::P& s) : function(f), shift(s) { }

    virtual typename D::I getDomain() const override {
        const auto& domain = function->getDomain();
        typename D::I i(domain.getLower() + shift, domain.getUpper() + shift, domain.getLowerClosed(), domain.getLowerClosed(), domain.getFixed());
        return i;
    }

    virtual R getValue(const typename D::P& p) const override {
        return function->getValue(p - shift);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function->partition(i.getShifted(-shift), [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1))
                callback(i1.getShifted(shift), f1);
            else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                UnilinearFunction<R, D> g(i1.getLower() + shift, i1.getUpper() + shift, f1l->getValue(i1.getLower()), f1l->getValue(i1.getUpper()), f1l->getDimension());
                simplifyAndCall(i1.getShifted(shift), &g, callback);
            }
            else
                throw cRuntimeError("TODO");
        });
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Shift, s = " << shift << "\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

// TODO:
//template<typename R, typename D>
//class INET_API QuadraticFunction : public Function<R, D>
//{
//  protected:
//    const double a;
//    const double b;
//    const double c;
//
//  public:
//    QuadraticFunction(double a, double b, double c) : a(a), b(b), c(c) { }
//
//    virtual R getValue(const typename D::P& p) const override {
//        return R(0);
//    }
//
//    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
//        callback(i, this);
//    }
//};

/**
 * Reciprocal in a given dimension and constant in the others.
 */
template<typename R, typename D>
class INET_API UnireciprocalFunction : public FunctionBase<R, D>
{
  protected:
    // f(x) = (a * x + b) / (c * x + d)
    const double a;
    const double b;
    const double c;
    const double d;
    const int dimension;

  protected:
    virtual double getIntegralFunctionValue(const typename D::P& p) const {
        // https://www.wolframalpha.com/input/?i=integrate+(a+*+x+%2B+b)+%2F+(c+*+x+%2B+d)
        double x = p.get(dimension);
        return (a * c * x + (b * c - a * d) * std::log(d + c * x)) / (c * c);
    }

  public:
    UnireciprocalFunction(double a, double b, double c, double d, int dimension) : a(a), b(b), c(c), d(d), dimension(dimension) { }

    virtual int getDimension() const { return dimension; }

    virtual R getValue(const typename D::P& p) const override {
        double x = p.get(dimension);
        return R(a * x + b) / (c * x + d);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual R getMin(const typename D::I& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getLowerBound<R>();
        else
            return std::min(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const typename D::I& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getUpperBound<R>();
        else
            return std::max(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMean(const typename D::I& i) const override {
        return getIntegral(i) / (i.getUpper().get(dimension) - i.getLower().get(dimension));
    }

    virtual R getIntegral(const typename D::I& i) const override {
        return R(getIntegralFunctionValue(i.getUpper()) - getIntegralFunctionValue(i.getLower()));
    }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "reciprocal in dim. " << dimension << " over " << i << "\n"
           << std::string(level + 2, ' ') << "→ " << getValue(i.getLower()) << " … " << getValue(i.getUpper()) << std::endl;
    }
};

//template<typename R, typename D>
//class INET_API BireciprocalFunction : public FunctionBase<R, D>
//{
//  protected:
//    // f(x, y) = (a0 + a1 * x + a2 * y + a3 * x * y) / (b0 + b1 * x + b2 * y + b3 * x * y)
//    const double a0;
//    const double a1;
//    const double a2;
//    const double a3;
//    const double b0;
//    const double b1;
//    const double b2;
//    const double b3;
//    const int dimension1;
//    const int dimension2;
//
//  protected:
//    double getIntegral(const typename D::P& p) const {
//        double x = p.get(dimension1);
//        double y = p.get(dimension2);
//        throw cRuntimeError("TODO");
//    }
//
//  public:
//    BireciprocalFunction(double a0, double a1, double a2, double a3, double b0, double b1, double b2, double b3, int dimension1, int dimension2) :
//        a0(a0), a1(a1), a2(a2), a3(a3), b0(b0), b1(b1), b2(b2), b3(b3), dimension1(dimension1), dimension2(dimension2) { }
//
//    virtual R getValue(const typename D::P& p) const override {
//        double x = p.get(dimension1);
//        double y = p.get(dimension2);
//        return R((a0 + a1 * x + a2 * y + a3 * x * y) / (b0 + b1 * x + b2 * y + b3 * x * y));
//    }
//
//    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
//        callback(i, this);
//    }
//
//    virtual R getMin(const typename D::I& i) const override {
//        throw cRuntimeError("TODO");
//    }
//
//    virtual R getMax(const typename D::I& i) const override {
//        throw cRuntimeError("TODO");
//    }
//
//    virtual R getMean(const typename D::I& i) const override {
//        throw cRuntimeError("TODO");
//    }
//};

template<typename R, typename X>
class INET_API SawtoothFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X start;
    const X end;
    const X period;
    const R value;

  public:
    SawtoothFunction(X start, X end, X period, R value) :
        start(start), end(end), period(period), value(value)
    { }

    virtual R getValue(const Point<X>& p) const override {
        auto x = std::get<0>(p);
        if (x < start)
            return R(0);
        else if (x > end)
            return R(0);
        else
            return value * fmod(toDouble(x - start) / toDouble(period), 1);
    }
};

template<typename R, typename D>
class INET_API AdditionFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    AdditionFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) + function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 + x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0) {
                    function2->partition(i1, callback);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                // TODO: use template specialization for compile time optimization
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() + f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) + f1c->getConstantValue(), f2l->getValue(i2.getUpper()) + f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) + f2c->getConstantValue(), f1l->getValue(i2.getUpper()) + f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) + f2l->getValue(i2.getLower()), f1l->getValue(i2.getUpper()) + f2l->getValue(i2.getUpper()), f1l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else {
                            typename D::P lowerLower = D::P::getZero();
                            typename D::P lowerUpper = D::P::getZero();
                            typename D::P upperLower = D::P::getZero();
                            typename D::P upperUpper = D::P::getZero();
                            lowerLower.set(f1l->getDimension(), i1.getLower().get(f1l->getDimension()));
                            lowerUpper.set(f1l->getDimension(), i1.getLower().get(f1l->getDimension()));
                            upperLower.set(f1l->getDimension(), i1.getUpper().get(f1l->getDimension()));
                            upperUpper.set(f1l->getDimension(), i1.getUpper().get(f1l->getDimension()));
                            lowerLower.set(f2l->getDimension(), i2.getLower().get(f2l->getDimension()));
                            lowerUpper.set(f2l->getDimension(), i2.getUpper().get(f2l->getDimension()));
                            upperLower.set(f2l->getDimension(), i2.getLower().get(f2l->getDimension()));
                            upperUpper.set(f2l->getDimension(), i2.getUpper().get(f2l->getDimension()));
                            R rLowerLower = f1l->getValue(lowerLower) + f2l->getValue(lowerLower);
                            R rLowerUpper = f1l->getValue(lowerUpper) + f2l->getValue(lowerUpper);
                            R rUpperLower = f1l->getValue(upperLower) + f2l->getValue(upperLower);
                            R rUpperUpper = f1l->getValue(upperUpper) + f2l->getValue(upperUpper);
                            BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, f1l->getDimension(), f2l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(+ ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SubtractionFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    SubtractionFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) - function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() - f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) - f1c->getConstantValue(), f2l->getValue(i2.getUpper()) - f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) - f2c->getConstantValue(), f1l->getValue(i2.getUpper()) - f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) - f2l->getValue(i2.getLower()), f1l->getValue(i2.getUpper()) - f2l->getValue(i2.getUpper()), f1l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(- ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API MultiplicationFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<double, D>> function2;

  public:
    MultiplicationFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<double, D>>& function2) :
        function1(function1), function2(function2)
    { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual const Ptr<const IFunction<R, D>>& getF1() const { return function1; }

    virtual const Ptr<const IFunction<double, D>>& getF2() const { return function2; }

    virtual R getValue(const typename D::P& p) const override {
        return function1->getValue(p) * function2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 * x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0 && function2->isFinite(i1)) {
                    callback(i1, f1);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<double, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<double, D> *>(f2)) {
                        ConstantFunction<R, D> g(f1c->getConstantValue() * f2c->getConstantValue());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<double, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f2l->getValue(i2.getLower()) * f1c->getConstantValue(), f2l->getValue(i2.getUpper()) * f1c->getConstantValue(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<double, D> *>(f2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), f1l->getValue(i2.getLower()) * f2c->getConstantValue(), f1l->getValue(i2.getUpper()) * f2c->getConstantValue(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<double, D> *>(f2)) {
                        // QuadraticFunction<double, D> g();
                        throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function1->isFinite(i) & function2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(* ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API DivisionFunction : public FunctionBase<double, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function1;
    const Ptr<const IFunction<R, D>> function2;

  public:
    DivisionFunction(const Ptr<const IFunction<R, D>>& function1, const Ptr<const IFunction<R, D>>& function2) : function1(function1), function2(function2) { }

    virtual typename D::I getDomain() const override {
        return function1->getDomain().getIntersected(function2->getDomain());
    }

    virtual double getValue(const typename D::P& p) const override {
        return unit(function1->getValue(p) / function2->getValue(p)).get();
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<double, D> *)> callback) const override {
        function1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            // NOTE: optimization for 0 / x
            if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                if (toDouble(f1c->getConstantValue()) == 0 && function2->isNonZero(i1)) {
                    ConstantFunction<double, D> g(0);
                    callback(i1, &g);
                    return;
                }
            }
            function2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        ConstantFunction<double, D> g(unit(f1c->getConstantValue() / f2c->getConstantValue()).get());
                        callback(i2, &g);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        UnireciprocalFunction<double, D> g(0, toDouble(f1c->getConstantValue()), f2l->getA(), f2l->getB(), f2l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                        UnilinearFunction<double, D> g(i2.getLower(), i2.getUpper(), unit(f1l->getValue(i2.getLower()) / f2c->getConstantValue()).get(), unit(f1l->getValue(i2.getUpper()) / f2c->getConstantValue()).get(), f1l->getDimension());
                        simplifyAndCall(i2, &g, callback);
                    }
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                        if (f1l->getDimension() == f2l->getDimension()) {
                            UnireciprocalFunction<double, D> g(f1l->getA(), f1l->getB(), f2l->getA(), f2l->getB(), f2l->getDimension());
                            simplifyAndCall(i2, &g, callback);
                        }
                        else {
                            throw cRuntimeError("TODO");
                            // BireciprocalFunction<double, D> g(...);
                            // simplifyAndCall(i2, &g, f);
                        }
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }

    virtual bool isFinite(const typename D::I& i) const override { return false; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(/ ";
        function1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        function2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SumFunction : public FunctionBase<R, D>
{
  protected:
    std::vector<Ptr<const IFunction<R, D>>> functions;

  public:
    SumFunction() { }
    SumFunction(const std::vector<Ptr<const IFunction<R, D>>>& functions) : functions(functions) { }

    const std::vector<Ptr<const IFunction<R, D>>>& getElements() const { return functions; }

    virtual void addElement(const Ptr<const IFunction<R, D>>& f) {
        functions.push_back(f);
    }

    virtual void removeElement(const Ptr<const IFunction<R, D>>& f) {
        functions.erase(std::remove(functions.begin(), functions.end(), f), functions.end());
    }

    virtual typename D::I getDomain() const override {
        typename D::I domain(D::P::getLowerBounds(), D::P::getUpperBounds(), 0, 0, 0);
        for (auto f : functions)
            domain = domain.getIntersected(f->getDomain());
        return domain;
    }

    virtual R getValue(const typename D::P& p) const override {
        R sum = R(0);
        for (auto f : functions)
            sum += f->getValue(p);
        return sum;
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        ConstantFunction<R, D> g(R(0));
        partition(0, i, callback, &g);
    }

    virtual void partition(int index, const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback, const IFunction<R, D> *f) const {
        if (index == (int)functions.size())
            simplifyAndCall(i, f, callback);
        else {
            functions[index]->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
                if (auto fc = dynamic_cast<const ConstantFunction<R, D> *>(f)) {
                    if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                        ConstantFunction<R, D> g(fc->getConstantValue() + f1c->getConstantValue());
                        partition(index + 1, i1, callback, &g);
                    }
                    else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                        UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), f1l->getValue(i1.getLower()) + fc->getConstantValue(), f1l->getValue(i1.getUpper()) + fc->getConstantValue(), f1l->getDimension());
                        partition(index + 1, i1, callback, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto fl = dynamic_cast<const UnilinearFunction<R, D> *>(f)) {
                    if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                        UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), fl->getValue(i1.getLower()) + f1c->getConstantValue(), fl->getValue(i1.getUpper()) + f1c->getConstantValue(), fl->getDimension());
                        partition(index + 1, i1, callback, &g);
                    }
                    else if (auto f1l = dynamic_cast<const UnilinearFunction<R, D> *>(f1)) {
                        if (fl->getDimension() == f1l->getDimension()) {
                            UnilinearFunction<R, D> g(i1.getLower(), i1.getUpper(), fl->getValue(i1.getLower()) + f1l->getValue(i1.getLower()), fl->getValue(i1.getUpper()) + f1l->getValue(i1.getUpper()), fl->getDimension());
                            partition(index + 1, i1, callback, &g);
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
    }

    virtual bool isFinite(const typename D::I& i) const override {
        for (auto f : functions)
            if (!f->isFinite(i))
                return false;
        return true;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Σ ";
        bool first = true;
        for (auto f : functions) {
            if (first)
                first = false;
            else
                os << "\n" << std::string(level + 3, ' ');
            f->printStructure(os, level + 3);
        }
        os << ")";
    }
};

template<typename R, typename X, typename Y>
class INET_API ModulatedFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<X, Y>>> function;
    const Ptr<const IFunction<Y, Domain<X>>> modulator;

  public:
    ModulatedFunction(const Ptr<const IFunction<R, Domain<X, Y>>>& function, const Ptr<const IFunction<Y, Domain<X>>>& modulator) :
        function(function), modulator(modulator) { }

    virtual R getValue(const Point<X, Y>& p) const override {
        auto x = std::get<0>(p);
        auto y = std::get<1>(p);
        auto mv = modulator->getValue({x});
        return function->getValue({x, y - mv});
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
        modulator->partition(i.template get<X, 0>(), [&] (const Interval<X>& i1, const IFunction<Y, Domain<X>> *f1) {
            if (auto f1c = dynamic_cast<const ConstantFunction<Y, Domain<X>> *>(f1)) {
                Point<X, Y> s(X(0), f1c->getConstantValue());
                function->partition(i.template getReplaced<X, 0>(i1).getShifted(-s), [&] (const Interval<X, Y>& i2, const IFunction<R, Domain<X, Y>> *f2) {
                    if (auto f2c = dynamic_cast<const ConstantFunction<R, Domain<X, Y>> *>(f2))
                        callback(i2.getShifted(s), f2);
                    else if (auto f2l = dynamic_cast<const UnilinearFunction<R, Domain<X, Y>> *>(f2)) {
                        UnilinearFunction<R, Domain<X, Y>> g(i2.getLower() + s, i2.getUpper() + s, f2l->getValue(i2.getLower()), f2l->getValue(i2.getUpper()), f2l->getDimension());
                        simplifyAndCall(i2.getShifted(s), &g, callback);
                    }
                    else
                        throw cRuntimeError("TODO");
                });
            }
            else
                throw cRuntimeError("TODO");
        });
    }
};

template<typename R, typename X, typename Y, int DIMS, typename RI>
class INET_API IntegratedFunction<R, Domain<X, Y>, DIMS, RI, Domain<X>> : public FunctionBase<RI, Domain<X>>
{
    const Ptr<const IFunction<R, Domain<X, Y>>> function;

  public:
    IntegratedFunction(const Ptr<const IFunction<R, Domain<X, Y>>>& function): function(function) { }

    virtual RI getValue(const Point<X>& p) const override {
        Point<X, Y> l1(std::get<0>(p), getLowerBound<Y>());
        Point<X, Y> u1(std::get<0>(p), getUpperBound<Y>());
        RI ri(0);
        Interval<X, Y> i1(l1, u1, DIMS, DIMS, DIMS);
        function->partition(i1, [&] (const Interval<X, Y>& i2, const IFunction<R, Domain<X, Y>> *f2) {
            R r = f2->getIntegral(i2);
            ri += RI(toDouble(r));
        });
        return ri;
    }

    virtual void partition(const Interval<X>& i, std::function<void (const Interval<X>&, const IFunction<RI, Domain<X>> *)> callback) const override {
        Point<X, Y> l1(std::get<0>(i.getLower()), getLowerBound<Y>());
        Point<X, Y> u1(std::get<0>(i.getUpper()), getUpperBound<Y>());
        Interval<X, Y> i1(l1, u1, (i.getLowerClosed() & 0b1) << 1, (i.getUpperClosed() & 0b1) << 1, (i.getFixed() & 0b1) << 1);
        std::set<X> xs;
        function->partition(i1, [&] (const Interval<X, Y>& i2, const IFunction<R, Domain<X, Y>> *f2) {
            xs.insert(std::get<0>(i2.getLower()));
            xs.insert(std::get<0>(i2.getUpper()));
        });
        bool first = true;
        X xLower;
        for (auto it : xs) {
            X xUpper = it;
            if (first)
                first = false;
            else {
                RI ri(0);
                // NOTE: use the lower X for both interval ends, because we assume a constant function and intervals are closed at the lower end
                Point<X, Y> l3(xLower, getLowerBound<Y>());
                Point<X, Y> u3(xLower, getUpperBound<Y>());
                Interval<X, Y> i3(l3, u3, DIMS, DIMS, DIMS);
                function->partition(i3, [&] (const Interval<X, Y>& i4, const IFunction<R, Domain<X, Y>> *f4) {
                    if (dynamic_cast<const ConstantFunction<R, Domain<X, Y>> *>(f4)) {
                        R r = f4->getIntegral(i4);
                        ri += RI(toDouble(r));
                    }
                    else if (auto f4l = dynamic_cast<const UnilinearFunction<R, Domain<X, Y>> *>(f4)) {
                        if (f4l->getDimension() == 1) {
                            R r = f4->getIntegral(i4);
                            ri += RI(toDouble(r));
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                });
                ConstantFunction<RI, Domain<X>> g(ri);
                Point<X> l5(xLower);
                Point<X> u5(xUpper);
                Interval<X> i5(l5, u5, first ? (i.getLowerClosed() & 0b10) >> 1 : 0b1, 0b0, 0b0);
                callback(i5, &g);
            }
            xLower = xUpper;
        }
    }
};

template<typename R, typename D, int DIMS, typename RI, typename DI>
class INET_API IntegratedFunction : public FunctionBase<RI, DI>
{
    const Ptr<const IFunction<R, D>> function;

  public:
    IntegratedFunction(const Ptr<const IFunction<R, D>>& function): function(function) { }

    virtual RI getValue(const typename DI::P& p) const override {
        auto l1 = D::P::getLowerBounds();
        auto u1 = D::P::getUpperBounds();
        p.template copyTo<typename D::P, DIMS>(l1);
        p.template copyTo<typename D::P, DIMS>(u1);
        RI ri(0);
        typename D::I i1(l1, u1, DIMS, DIMS, DIMS);
        function->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
            R r = f2->getIntegral(i2);
            ri += RI(toDouble(r));
        });
        return ri;
    }

    virtual void partition(const typename DI::I& i, std::function<void (const typename DI::I&, const IFunction<RI, DI> *)> callback) const override {
        throw cRuntimeError("TODO");
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Integrated\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename D, int DIMENSION, typename X>
class INET_API ApproximatedFunction : public FunctionBase<R, D>
{
  protected:
    const X lower;
    const X upper;
    const X step;
    const IInterpolator<X, R> *interpolator;
    const Ptr<const IFunction<R, D>> function;

  public:
    ApproximatedFunction(X lower, X upper, X step, const IInterpolator<X, R> *interpolator, const Ptr<const IFunction<R, D>>& function) :
        lower(lower), upper(upper), step(step), interpolator(interpolator), function(function)
    { }

    virtual R getValue(const typename D::P& p) const override {
        X x = std::get<DIMENSION>(p);
        if (x < lower)
            return function->getValue(p.template getReplaced<X, DIMENSION>(lower));
        else if (x > upper)
            return function->getValue(p.template getReplaced<X, DIMENSION>(upper));
        else {
            X x1 = lower + step * floor(toDouble(x - lower) / toDouble(step));
            X x2 = std::min(upper, x1 + step);
            R r1 = function->getValue(p.template getReplaced<X, DIMENSION>(x1));
            R r2 = function->getValue(p.template getReplaced<X, DIMENSION>(x2));
            return interpolator->getValue(x1, r1, x2, r2, x);
        }
    }

    virtual void partition(const typename D::I& i, std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        unsigned char b = 1 << std::tuple_size<typename D::P::type>::value >> 1;
        auto m = (b >> DIMENSION);
        auto fixed = i.getFixed() & m;
        const auto& pl = i.getLower();
        const auto& pu = i.getUpper();
        X xl = std::get<DIMENSION>(pl);
        X xu = std::get<DIMENSION>(pu);
        if (xl < lower) {
            function->partition(i.template getFixed<X, DIMENSION>(lower), [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    auto p1 = i1.getLower().template getReplaced<X, DIMENSION>(xl);
                    auto p2 = i1.getUpper().template getReplaced<X, DIMENSION>(std::min(lower, xu));
                    ConstantFunction<R, D> g(f1c->getConstantValue());
                    typename D::I i2(p1, p2, i.getLowerClosed() | fixed, (i.getUpperClosed() & ~m) | fixed, i.getFixed());
                    callback(i2, &g);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
        if (xu >= lower && upper >= xl) {
            X min = lower + step * floor(toDouble(std::max(lower, xl) - lower) / toDouble(step));
            X max = (i.getFixed() & m) ? min + step : lower + step * ceil(toDouble(std::min(upper, xu) - lower) / toDouble(step));
            for (X x = min; x < max; x += step) {
                X x1 = x;
                X x2 = x + step;
                Interval<X> j(std::max(x1, xl), std::min(x2, xu), true, fixed, fixed);
                // determine the smallest intervals along the other dimensions for which the primitive functions are calculated
                function->partition(i.template getFixed<X, DIMENSION>(x1), [&] (const typename D::I& i2, const IFunction<R, D> *f2) {
                    function->partition(i2.template getFixed<X, DIMENSION>(x2), [&] (const typename D::I& i3, const IFunction<R, D> *f3) {
                        auto i4 = i3.template getReplaced<X, DIMENSION>(j);
                        if (auto f2c = dynamic_cast<const ConstantFunction<R, D> *>(f2)) {
                            auto r1 = f2c->getConstantValue();
                            if (auto f3c = dynamic_cast<const ConstantFunction<R, D> *>(f3)) {
                                auto r2 = f3c->getConstantValue();
                                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator)) {
                                    ConstantFunction<R, D> g(interpolator->getValue(x1, r1, x2, r2, (x1 + x2) / 2));
                                    callback(i4, &g);
                                }
                                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                                    auto p3 = i3.getLower().template getReplaced<X, DIMENSION>(x1);
                                    auto p4 = i3.getUpper().template getReplaced<X, DIMENSION>(x2);
                                    UnilinearFunction<R, D> g(p3, p4, r1, r2, DIMENSION);
                                    simplifyAndCall(i4, &g, callback);
                                }
                                else
                                    throw cRuntimeError("TODO");
                            }
                            else if (auto f3l = dynamic_cast<const UnilinearFunction<R, D> *>(f3)) {
                                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator))
                                    throw cRuntimeError("TODO");
                                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                                    typename D::P lowerLower = i4.getLower();
                                    typename D::P lowerUpper = i4.getUpper();
                                    typename D::P upperLower = i3.getLower().template getReplaced<X, DIMENSION>(x1);
                                    typename D::P upperUpper = i3.getUpper().template getReplaced<X, DIMENSION>(x2);
                                    R rLowerLower = f2c->getConstantValue();
                                    R rLowerUpper = f2c->getConstantValue();
                                    R rUpperLower = f3l->getRLower();
                                    R rUpperUpper = f3l->getRUpper();
                                    BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, f3l->getDimension(), DIMENSION);
                                    simplifyAndCall(i4, &g, callback);
                                }
                                else
                                    throw cRuntimeError("TODO");
                            }
                            else
                                throw cRuntimeError("TODO");
                        }
                        else if (auto f2l = dynamic_cast<const UnilinearFunction<R, D> *>(f2)) {
                            if (auto f3c = dynamic_cast<const ConstantFunction<R, D> *>(f3)) {
                                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator))
                                    throw cRuntimeError("TODO");
                                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                                    typename D::P lowerLower = i2.getLower().template getReplaced<X, DIMENSION>(x1);
                                    typename D::P lowerUpper = i2.getUpper().template getReplaced<X, DIMENSION>(x2);
                                    typename D::P upperLower = i4.getLower();
                                    typename D::P upperUpper = i4.getUpper();
                                    R rLowerLower = f2l->getRLower();
                                    R rLowerUpper = f2l->getRUpper();
                                    R rUpperLower = f3c->getConstantValue();
                                    R rUpperUpper = f3c->getConstantValue();
                                    BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, f2l->getDimension(), DIMENSION);
                                    simplifyAndCall(i4, &g, callback);
                                }
                                else
                                    throw cRuntimeError("TODO");
                            }
                            else if (auto f3l = dynamic_cast<const UnilinearFunction<R, D> *>(f3)) {
                                ASSERT(f2l->getDimension() == f3l->getDimension());
                                typename D::P lowerLower = i2.getLower().template getReplaced<X, DIMENSION>(x1);
                                typename D::P lowerUpper = i2.getUpper().template getReplaced<X, DIMENSION>(x2);
                                typename D::P upperLower = i3.getLower().template getReplaced<X, DIMENSION>(x1);
                                typename D::P upperUpper = i3.getUpper().template getReplaced<X, DIMENSION>(x2);
                                R rLowerLower = f2l->getRLower();
                                R rLowerUpper = f2l->getRUpper();
                                R rUpperLower = f3l->getRLower();
                                R rUpperUpper = f3l->getRUpper();
                                BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, f2l->getDimension(), DIMENSION);
                                simplifyAndCall(i4, &g, callback);
                            }
                            else
                                throw cRuntimeError("TODO");
                        }
                        else
                            throw cRuntimeError("TODO");
                    });
                });
            }
        }
        if (xu > upper) {
            function->partition(i.template getFixed<X, DIMENSION>(upper), [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
                if (auto f1c = dynamic_cast<const ConstantFunction<R, D> *>(f1)) {
                    auto p1 = i1.getLower().template getReplaced<X, DIMENSION>(std::max(upper, xl));
                    auto p2 = i1.getUpper().template getReplaced<X, DIMENSION>(xu);
                    ConstantFunction<R, D> g(f1c->getConstantValue());
                    typename D::I i2(p1, p2, i.getLowerClosed() | m, (i.getUpperClosed() & ~m) | fixed, i.getFixed());
                    callback(i2, &g);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return function->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Approximated\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename X, typename Y>
class INET_API ExtrudedFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<Y>>> function;

  public:
    ExtrudedFunction(const Ptr<const IFunction<R, Domain<Y>>>& function) : function(function) { }

    virtual R getValue(const Point<X, Y>& p) const override {
        return function->getValue(Point<Y>(std::get<1>(p)));
    }

    virtual void partition(const Interval<X, Y>& i, std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
        Interval<Y> i1(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), i.getUpperClosed() & 0b01);
        function->partition(i1, [&] (const Interval<Y>& i2, const IFunction<R, Domain<Y>> *f2) {
            Point<X, Y> lower(std::get<0>(i.getLower()), std::get<0>(i2.getLower()));
            Point<X, Y> upper(std::get<0>(i.getUpper()), std::get<0>(i2.getUpper()));
            Interval<X, Y> i3(lower, upper, (i.getUpperClosed() & 0b10) | i2.getUpperClosed());
            if (auto f2c = dynamic_cast<const ConstantFunction<R, Domain<Y>> *>(f2)) {
                ConstantFunction<R, Domain<X, Y>> g(f2c->getConstantValue());
                callback(i3, &g);
            }
            else if (auto f2l = dynamic_cast<const UnilinearFunction<R, Domain<Y>> *>(f2)) {
                Point<X, Y> lower(std::get<0>(i.getLower()), std::get<0>(f2l->getLower()));
                Point<X, Y> upper(std::get<0>(i.getUpper()), std::get<0>(f2l->getUpper()));
                UnilinearFunction<R, Domain<X, Y>> g(lower, upper, f2l->getRLower(), f2l->getRUpper(), 1);
                callback(i3, &g);
            }
            else
                throw cRuntimeError("TODO");
        });
    }

    virtual bool isFinite(const Interval<X, Y>& i) const override {
        Interval<Y> i1(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), i.getUpperClosed() & 0b01);
        return function->isFinite(i1);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Extruded\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API MemoizedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function;
    const int limit;
    mutable std::map<typename D::P, R> cache;

  public:
    MemoizedFunction(const Ptr<const IFunction<R, D>>& function, int limit = INT_MAX) :
        function(function), limit(limit) { }

    virtual R getValue(const typename D::P& p) const override {
        auto it = cache.find(p);
        if (it != cache.end())
            return it->second;
        else {
            R v = function->getValue(p);
            cache[p] = v;
            if ((int)cache.size() > limit)
                cache.clear();
            return v;
        }
    }

    virtual void partition(const typename D::I& i, std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function->partition(i, callback);
    }
};

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const IFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) {
    callback(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const UnilinearFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) {
    if (f->getRLower() == f->getRUpper()) {
        ConstantFunction<R, D> g(f->getRLower());
        callback(i, &g);
    }
    else
        callback(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const BilinearFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) {
    if (f->getRLowerLower() == f->getRLowerUpper() && f->getRLowerLower() == f->getRUpperLower() && f->getRLowerLower() == f->getRUpperUpper()) {
        ConstantFunction<R, D> g(f->getRLowerLower());
        callback(i, &g);
    }
    // TODO: one dimensional linear functions?
    else
        callback(i, f);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_FUNCTIONS_H_

