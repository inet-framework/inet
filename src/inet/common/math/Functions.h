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
    const Ptr<const IFunction<R, D>> f;

  public:
    FunctionChecker(const Ptr<const IFunction<R, D>>& f) : f(f) { }

    void check() const {
        check(f->getDomain());
    }

    void check(const typename D::I& i) const {
        f->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *g) {
            auto check = std::function<void (const typename D::P&)>([&] (const typename D::P& p) {
                if (i1.contains(p)) {
                    R rF = f->getValue(p);
                    R rG = g->getValue(p);
                    ASSERT(rF == rG || (std::isnan(toDouble(rF)) && std::isnan(toDouble(rG))));
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
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f) {
            result &= f->isFinite(i1);
        });
        return result;
    }

    virtual R getMin() const override { return getMin(getDomain()); }
    virtual R getMin(const typename D::I& i) const override {
        R result(getUpperBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f) {
            result = std::min(f->getMin(i1), result);
        });
        return result;
    }

    virtual R getMax() const override { return getMax(getDomain()); }
    virtual R getMax(const typename D::I& i) const override {
        R result(getLowerBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f) {
            result = std::max(f->getMax(i1), result);
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
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f) {
            double volume = i1.getVolume();
            R value = f->getMean(i1);
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
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *g) {
            os << std::string(level, ' ');
            g->printPartition(os, i1, level);
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
    const Ptr<const IFunction<R, D>> f;
    const Interval<R> range;
    const typename D::I domain;

  public:
    DomainLimitedFunction(const Ptr<const IFunction<R, D>>& f, const typename D::I& domain) : f(f), range(Interval<R>(f->getMin(domain), f->getMax(domain), 0b1, 0b1, 0b0)), domain(domain) { }

    virtual Interval<R> getRange() const override { return range; }
    virtual typename D::I getDomain() const override { return domain; }

    virtual R getValue(const typename D::P& p) const override {
        ASSERT(domain.contains(p));
        return f->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> g) const override {
        const auto& i1 = i.intersect(domain);
        if (!i1.isEmpty())
            f->partition(i1, g);
    }

    virtual R getMin(const typename D::I& i) const override {
        return f->getMin(i.intersect(domain));
    }

    virtual R getMax(const typename D::I& i) const override {
        return f->getMax(i.intersect(domain));
    }

    virtual R getMean(const typename D::I& i) const override {
        return f->getMean(i.intersect(domain));
    }

    virtual R getIntegral(const typename D::I& i) const override {
        return f->getIntegral(i.intersect(domain));
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(DomainLimited, domain = " << domain << "\n" << std::string(level + 2, ' ');
        f->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename D>
Ptr<const DomainLimitedFunction<R, D>> makeFirstQuadrantLimitedFunction(const Ptr<const IFunction<R, D>>& f) {
    auto mask = (1 << std::tuple_size<typename D::P::type>::value) - 1;
    typename D::I i(D::P::getZero(), D::P::getUpperBounds(), mask, 0, 0);
    return makeShared<DomainLimitedFunction<R, D>>(f, i);
}

template<typename R, typename D>
class INET_API ConstantFunction : public FunctionBase<R, D>
{
  protected:
    const R r;

  public:
    ConstantFunction(R r) : r(r) { }

    virtual R getConstantValue() const { return r; }

    virtual Interval<R> getRange() const override { return Interval<R>(r, r, 0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override { return r; }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override { return std::isfinite(toDouble(r)); }
    virtual R getMin(const typename D::I& i) const override { return r; }
    virtual R getMax(const typename D::I& i) const override { return r; }
    virtual R getMean(const typename D::I& i) const override { return r; }
    virtual R getIntegral(const typename D::I& i) const override { return r == R(0) ? r : r * i.getVolume(); }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "constant over " << i << "\n" << std::string(level + 2, ' ') << "→ " << r << std::endl;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "Constant " << r;
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
    const R r;

  public:
    OneDimensionalBoxcarFunction(X lower, X upper, R r) : lower(lower), upper(upper), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), r, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X>& p) const override {
        return std::get<0>(p) < lower || std::get<0>(p) >= upper ? R(0) : r;
    }

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, Domain<X>> *)> f) const override {
        const auto& i1 = i.intersect(Interval<X>(getLowerBound<X>(), Point<X>(lower), 0b0, 0b0, 0b0));
        if (!i1.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            f(i1, &g);
        }
        const auto& i2 = i.intersect(Interval<X>(Point<X>(lower), Point<X>(upper), 0b1, 0b0, 0b0));
        if (!i2.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(r);
            f(i2, &g);
        }
        const auto& i3 = i.intersect(Interval<X>(Point<X>(upper), getUpperBound<X>(), 0b1, 0b0, 0b0));
        if (!i3.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            f(i3, &g);
        }
    }

    virtual bool isFinite(const Interval<X>& i) const override { return std::isfinite(toDouble(r)); }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(OneDimensionalBoxcar, [" << lower << " … " << upper << "] → " << r << ")";
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
    const R r;

  protected:
    void callf(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> f, R r) const {
        if (!i.isEmpty()) {
            ConstantFunction<R, Domain<X, Y>> g(r);
            f(i, &g);
        }
    }

  public:
    TwoDimensionalBoxcarFunction(X lowerX, X upperX, Y lowerY, Y upperY, R r) : lowerX(lowerX), upperX(upperX), lowerY(lowerY), upperY(upperY), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), r, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X, Y>& p) const override {
        return std::get<0>(p) < lowerX || std::get<0>(p) >= upperX || std::get<1>(p) < lowerY || std::get<1>(p) >= upperY ? R(0) : r;
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> f) const override {
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), getLowerBound<Y>()), Point<X, Y>(X(lowerX), Y(lowerY)), 0b00, 0b00, 0b00)), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), getLowerBound<Y>()), Point<X, Y>(X(upperX), Y(lowerY)), 0b10, 0b00, 0b00)), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), getLowerBound<Y>()), Point<X, Y>(getUpperBound<X>(), Y(lowerY)), 0b10, 0b00, 0b00)), f, R(0));

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(lowerY)), Point<X, Y>(X(lowerX), Y(upperY)), 0b01, 0b00, 0b00)), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(lowerY)), Point<X, Y>(X(upperX), Y(upperY)), 0b11, 0b00, 0b00)), f, r);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(lowerY)), Point<X, Y>(getUpperBound<X>(), Y(upperY)), 0b11, 0b00, 0b00)), f, R(0));

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(upperY)), Point<X, Y>(X(lowerX), getUpperBound<Y>()), 0b01, 0b00, 0b00)), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(upperY)), Point<X, Y>(X(upperX), getUpperBound<Y>()), 0b11, 0b00, 0b00)), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(upperY)), Point<X, Y>(getUpperBound<X>(), getUpperBound<Y>()), 0b11, 0b00, 0b00)), f, R(0));
    }

    virtual bool isFinite(const Interval<X, Y>& i) const override { return std::isfinite(toDouble(r)); }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(TwoDimensionalBoxcar, [" << lowerX << " … " << upperX << "] x [" << lowerY << " … " << upperY << "] → " << r << ")";
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
    UnilinearFunction(typename D::P lower, typename D::P upper, R rLower, R rUpper, int dimension) : lower(lower), upper(upper), rLower(rLower), rUpper(rUpper), dimension(dimension) { }

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

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f(i, this);
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

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f(i, this);
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

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, Domain<X>> *)> f) const override {
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
            auto i1 = i.intersect(Interval<X>(Point<X>(it->first), Point<X>(jt->first), 0b1, kt == rs.end() ? 0b1 : 0b0, 0b0));
            if (!i1.isEmpty()) {
                const auto interpolator = it->second.second;
                auto xLower = std::get<0>(i1.getLower());
                auto xUpper = std::get<0>(i1.getUpper());
                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator)) {
                    auto value = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, (xLower + xUpper) / 2);
                    ConstantFunction<R, Domain<X>> g(value);
                    f(i1, &g);
                }
                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                    auto yLower = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, xLower);
                    auto yUpper = interpolator->getValue(it->first, it->second.first, jt->first, jt->second.first, xUpper);
                    UnilinearFunction<R, Domain<X>> g(xLower, xUpper, yLower, yUpper, 0);
                    simplifyAndCall(i1, &g, f);
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
//    const IInterpolator<T, R>& xi;
//    const IInterpolator<T, R>& yi;
//    const std::vector<std::tuple<X, Y, R>> rs;
//
//  public:
//    TwoDimensionalInterpolatedFunction(const IInterpolator<T, R>& xi, const IInterpolator<T, R>& yi, const std::vector<std::tuple<X, Y, R>>& rs) :
//        xi(xi), yi(yi), rs(rs) { }
//
//    virtual R getValue(const Point<T>& p) const override {
//        throw cRuntimeError("TODO");
//    }
//
//    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> f) const override {
//        throw cRuntimeError("TODO");
//    }
//};

//template<typename R, typename D0, typename D>
//class INET_API FunctionInterpolatingFunction : public Function<R, D>
//{
//  protected:
//    const IInterpolator<R, D0>& i;
//    const std::map<D0, const IFunction<R, D> *> fs;
//
//  public:
//    FunctionInterpolatingFunction(const IInterpolator<R, D0>& i, const std::map<D0, const IFunction<R, D> *>& fs) : i(i), fs(fs) { }
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
//    virtual void partition(const Interval<D0, D>& i, const std::function<void (const Interval<D0, D>&, const IFunction<R, D> *)> f) const override {
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

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, Domain<X>> *)> f) const override {
        throw cRuntimeError("Invalid operation");
    }
};

/**
 * Combines 2 one-dimensional functions into a two-dimensional function.
 */
template<typename R, typename X, typename Y>
class INET_API OrthogonalCombinatorFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<X>>> f;
    const Ptr<const IFunction<double, Domain<Y>>> g;

  public:
    OrthogonalCombinatorFunction(const Ptr<const IFunction<R, Domain<X>>>& f, const Ptr<const IFunction<double, Domain<Y>>>& g) : f(f), g(g) { }

    virtual Interval<X, Y> getDomain() const override {
        const auto& fDomain = f->getDomain();
        const auto& gDomain = g->getDomain();
        Point<X, Y> lower(std::get<0>(fDomain.getLower()), std::get<0>(gDomain.getLower()));
        Point<X, Y> upper(std::get<0>(fDomain.getUpper()), std::get<0>(gDomain.getUpper()));
        auto lowerClosed = fDomain.getLowerClosed() << 1 | gDomain.getLowerClosed();
        auto upperClosed = fDomain.getUpperClosed() << 1 | gDomain.getUpperClosed();
        auto fixed = fDomain.getFixed() << 1 | gDomain.getFixed();
        return Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed);
    }

    virtual R getValue(const Point<X, Y>& p) const override {
        return f->getValue(std::get<0>(p)) * g->getValue(std::get<1>(p));
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> h) const override {
        Interval<X> ix(Point<X>(std::get<0>(i.getLower())), Point<X>(std::get<0>(i.getUpper())), (i.getLowerClosed() & 0b10) >> 1, (i.getUpperClosed() & 0b10) >> 1, (i.getFixed() & 0b10) >> 1);
        Interval<Y> iy(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), (i.getLowerClosed() & 0b01) >> 0, (i.getUpperClosed() & 0b01) >> 0, (i.getFixed() & 0b01) >> 0);
        f->partition(ix, [&] (const Interval<X>& ixf, const IFunction<R, Domain<X>> *if1) {
            g->partition(iy, [&] (const Interval<Y>& iyg, const IFunction<double, Domain<Y>> *if2) {
                Point<X, Y> lower(std::get<0>(ixf.getLower()), std::get<0>(iyg.getLower()));
                Point<X, Y> upper(std::get<0>(ixf.getUpper()), std::get<0>(iyg.getUpper()));
                auto lowerClosed = (ixf.getLowerClosed() << 1) | (iyg.getLowerClosed() << 0);
                auto upperClosed = (ixf.getUpperClosed() << 1) | (iyg.getUpperClosed() << 0);
                auto fixed = (ixf.getFixed() << 1) | (iyg.getFixed() << 0);
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, Domain<X>> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, Domain<Y>> *>(if2)) {
                        ConstantFunction<R, Domain<X, Y>> g(cif1->getConstantValue() * cif2->getConstantValue());
                        h(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<double, Domain<Y>> *>(if2)) {
                        UnilinearFunction<R, Domain<X, Y>> g(lower, upper, lif2->getValue(iyg.getLower()) * cif1->getConstantValue(), lif2->getValue(iyg.getUpper()) * cif1->getConstantValue(), 1);
                        simplifyAndCall(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g, h);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const UnilinearFunction<R, Domain<X>> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, Domain<Y>> *>(if2)) {
                        UnilinearFunction<R, Domain<X, Y>> g(lower, upper, lif1->getValue(ixf.getLower()) * cif2->getConstantValue(), lif1->getValue(ixf.getUpper()) * cif2->getConstantValue(), 0);
                        simplifyAndCall(Interval<X, Y>(lower, upper, lowerClosed, upperClosed, fixed), &g, h);
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
        f->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        g->printStructure(os, level + 3);
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
    const Ptr<const IFunction<R, D>> f;
    const typename D::P s;

  public:
    ShiftFunction(const Ptr<const IFunction<R, D>>& f, const typename D::P& s) : f(f), s(s) { }

    virtual typename D::I getDomain() const override {
        const auto& domain = f->getDomain();
        typename D::I i(domain.getLower() + s, domain.getUpper() + s, domain.getLowerClosed(), domain.getLowerClosed(), domain.getFixed());
        return i;
    }

    virtual R getValue(const typename D::P& p) const override {
        return f->getValue(p - s);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> g) const override {
        f->partition(typename D::I(i.getLower() - s, i.getUpper() - s, i.getLowerClosed(), i.getUpperClosed(), i.getFixed()), [&] (const typename D::I& j, const IFunction<R, D> *jf) {
            if (auto cjf = dynamic_cast<const ConstantFunction<R, D> *>(jf))
                g(typename D::I(j.getLower() + s, j.getUpper() + s, j.getLowerClosed(), j.getUpperClosed(), j.getFixed()), jf);
            else if (auto ljf = dynamic_cast<const UnilinearFunction<R, D> *>(jf)) {
                UnilinearFunction<R, D> h(j.getLower() + s, j.getUpper() + s, ljf->getValue(j.getLower()), ljf->getValue(j.getUpper()), ljf->getDimension());
                simplifyAndCall(typename D::I(j.getLower() + s, j.getUpper() + s, j.getLowerClosed(), j.getUpperClosed(), j.getFixed()), &h, g);
            }
            else {
                ShiftFunction h(const_cast<IFunction<R, D> *>(jf)->shared_from_this(), s);
                simplifyAndCall(typename D::I(j.getLower() + s, j.getUpper() + s, j.getLowerClosed(), j.getUpperClosed(), j.getFixed()), &h, g);
            }
        });
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Shift, s = " << s << "\n" << std::string(level + 2, ' ');
        f->printStructure(os, level + 2);
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
//    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
//        throw cRuntimeError("TODO");
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

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f(i, this);
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
//    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
//        f(i, this);
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

template<typename R, typename D>
class INET_API AdditionFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> f1;
    const Ptr<const IFunction<R, D>> f2;

  public:
    AdditionFunction(const Ptr<const IFunction<R, D>>& f1, const Ptr<const IFunction<R, D>>& f2) : f1(f1), f2(f2) { }

    virtual typename D::I getDomain() const override {
        return f1->getDomain().intersect(f2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return f1->getValue(p) + f2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *if1) {
            f2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *if2) {
                // TODO: use template specialization for compile time optimization
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        ConstantFunction<R, D> g(cif1->getConstantValue() + cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) + cif1->getConstantValue(), lif2->getValue(i2.getUpper()) + cif1->getConstantValue(), lif2->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const UnilinearFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) + cif2->getConstantValue(), lif1->getValue(i2.getUpper()) + cif2->getConstantValue(), lif1->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        if (lif1->getDimension() == lif2->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) + lif2->getValue(i2.getLower()), lif1->getValue(i2.getUpper()) + lif2->getValue(i2.getUpper()), lif1->getDimension());
                            simplifyAndCall(i2, &g, f);
                        }
                        else {
                            typename D::P lowerLower = D::P::getZero();
                            typename D::P lowerUpper = D::P::getZero();
                            typename D::P upperLower = D::P::getZero();
                            typename D::P upperUpper = D::P::getZero();
                            lowerLower.set(lif1->getDimension(), i1.getLower().get(lif1->getDimension()));
                            lowerUpper.set(lif1->getDimension(), i1.getLower().get(lif1->getDimension()));
                            upperLower.set(lif1->getDimension(), i1.getUpper().get(lif1->getDimension()));
                            upperUpper.set(lif1->getDimension(), i1.getUpper().get(lif1->getDimension()));
                            lowerLower.set(lif2->getDimension(), i2.getLower().get(lif2->getDimension()));
                            lowerUpper.set(lif2->getDimension(), i2.getUpper().get(lif2->getDimension()));
                            upperLower.set(lif2->getDimension(), i2.getLower().get(lif2->getDimension()));
                            upperUpper.set(lif2->getDimension(), i2.getUpper().get(lif2->getDimension()));
                            R rLowerLower = lif1->getValue(lowerLower) + lif2->getValue(lowerLower);
                            R rLowerUpper = lif1->getValue(lowerUpper) + lif2->getValue(lowerUpper);
                            R rUpperLower = lif1->getValue(upperLower) + lif2->getValue(upperLower);
                            R rUpperUpper = lif1->getValue(upperUpper) + lif2->getValue(upperUpper);
                            BilinearFunction<R, D> g(lowerLower, lowerUpper, upperLower, upperUpper, rLowerLower, rLowerUpper, rUpperLower, rUpperUpper, lif1->getDimension(), lif2->getDimension());
                            simplifyAndCall(i2, &g, f);
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
        return f1->isFinite(i) & f2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(+ ";
        f1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        f2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SubtractionFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> f1;
    const Ptr<const IFunction<R, D>> f2;

  public:
    SubtractionFunction(const Ptr<const IFunction<R, D>>& f1, const Ptr<const IFunction<R, D>>& f2) : f1(f1), f2(f2) { }

    virtual typename D::I getDomain() const override {
        return f1->getDomain().intersect(f2->getDomain());
    }

    virtual R getValue(const typename D::P& p) const override {
        return f1->getValue(p) - f2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *if1) {
            f2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        ConstantFunction<R, D> g(cif1->getConstantValue() - cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) - cif1->getConstantValue(), lif2->getValue(i2.getUpper()) - cif1->getConstantValue(), lif2->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const UnilinearFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) - cif2->getConstantValue(), lif1->getValue(i2.getUpper()) - cif2->getConstantValue(), lif1->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        if (lif1->getDimension() == lif2->getDimension()) {
                            UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) - lif2->getValue(i2.getLower()), lif1->getValue(i2.getUpper()) - lif2->getValue(i2.getUpper()), lif1->getDimension());
                            simplifyAndCall(i2, &g, f);
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
        return f1->isFinite(i) & f2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(- ";
        f1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        f2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API MultiplicationFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> f1;
    const Ptr<const IFunction<double, D>> f2;

  public:
    MultiplicationFunction(const Ptr<const IFunction<R, D>>& f1, const Ptr<const IFunction<double, D>>& f2) : f1(f1), f2(f2) { }

    virtual typename D::I getDomain() const override {
        return f1->getDomain().intersect(f2->getDomain());
    }

    virtual const Ptr<const IFunction<R, D>>& getF1() const { return f1; }

    virtual const Ptr<const IFunction<double, D>>& getF2() const { return f2; }

    virtual R getValue(const typename D::P& p) const override {
        return f1->getValue(p) * f2->getValue(p);
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        f1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *if1) {
            // NOTE: optimization for 0 * x
            if (auto cif1 = dynamic_cast<const ConstantFunction<R, D> *>(if1)) {
                if (toDouble(cif1->getConstantValue()) == 0 && f2->isFinite(i1)) {
                    f(i1, if1);
                    return;
                }
            }
            f2->partition(i1, [&] (const typename D::I& i2, const IFunction<double, D> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, D> *>(if2)) {
                        ConstantFunction<R, D> g(cif1->getConstantValue() * cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<double, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) * cif1->getConstantValue(), lif2->getValue(i2.getUpper()) * cif1->getConstantValue(), lif2->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const UnilinearFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, D> *>(if2)) {
                        UnilinearFunction<R, D> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) * cif2->getConstantValue(), lif1->getValue(i2.getUpper()) * cif2->getConstantValue(), lif1->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<double, D> *>(if2)) {
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
        return f1->isFinite(i) & f2->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(* ";
        f1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        f2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API DivisionFunction : public FunctionBase<double, D>
{
  protected:
    const Ptr<const IFunction<R, D>> f1;
    const Ptr<const IFunction<R, D>> f2;

  public:
    DivisionFunction(const Ptr<const IFunction<R, D>>& f1, const Ptr<const IFunction<R, D>>& f2) : f1(f1), f2(f2) { }

    virtual typename D::I getDomain() const override {
        return f1->getDomain().intersect(f2->getDomain());
    }

    virtual double getValue(const typename D::P& p) const override {
        return unit(f1->getValue(p) / f2->getValue(p)).get();
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<double, D> *)> f) const override {
        f1->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *if1) {
            f2->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        ConstantFunction<double, D> g(unit(cif1->getConstantValue() / cif2->getConstantValue()).get());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        UnireciprocalFunction<double, D> g(0, toDouble(cif1->getConstantValue()), lif2->getA(), lif2->getB(), lif2->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const UnilinearFunction<R, D> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, D> *>(if2)) {
                        UnilinearFunction<double, D> g(i2.getLower(), i2.getUpper(), unit(lif1->getValue(i2.getLower()) / cif2->getConstantValue()).get(), unit(lif1->getValue(i2.getUpper()) / cif2->getConstantValue()).get(), lif1->getDimension());
                        simplifyAndCall(i2, &g, f);
                    }
                    else if (auto lif2 = dynamic_cast<const UnilinearFunction<R, D> *>(if2)) {
                        if (lif1->getDimension() == lif2->getDimension()) {
                            UnireciprocalFunction<double, D> g(lif1->getA(), lif1->getB(), lif2->getA(), lif2->getB(), lif2->getDimension());
                            simplifyAndCall(i2, &g, f);
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
        f1->printStructure(os, level + 3);
        os << "\n" << std::string(level + 3, ' ');
        f2->printStructure(os, level + 3);
        os << ")";
    }
};

template<typename R, typename D>
class INET_API SumFunction : public FunctionBase<R, D>
{
  protected:
    std::vector<Ptr<const IFunction<R, D>>> fs;

  public:
    SumFunction() { }
    SumFunction(const std::vector<Ptr<const IFunction<R, D>>>& fs) : fs(fs) { }

    const std::vector<Ptr<const IFunction<R, D>>>& getElements() const { return fs; }

    virtual void addElement(const Ptr<const IFunction<R, D>>& f) {
        fs.push_back(f);
    }

    virtual void removeElement(const Ptr<const IFunction<R, D>>& f) {
        fs.erase(std::remove(fs.begin(), fs.end(), f), fs.end());
    }

    virtual typename D::I getDomain() const override {
        typename D::I domain(D::P::getLowerBounds(), D::P::getUpperBounds(), 0, 0, 0);
        for (auto f : fs)
            domain = domain.intersect(f->getDomain());
        return domain;
    }

    virtual R getValue(const typename D::P& p) const override {
        R sum = R(0);
        for (auto f : fs)
            sum += f->getValue(p);
        return sum;
    }

    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f) const override {
        ConstantFunction<R, D> g(R(0));
        partition(0, i, f, &g);
    }

    virtual void partition(int index, const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> f, const IFunction<R, D> *g) const {
        if (index == (int)fs.size())
            simplifyAndCall(i, g, f);
        else {
            fs[index]->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *h) {
                if (auto cg = dynamic_cast<const ConstantFunction<R, D> *>(g)) {
                    if (auto ch = dynamic_cast<const ConstantFunction<R, D> *>(h)) {
                        ConstantFunction<R, D> j(cg->getConstantValue() + ch->getConstantValue());
                        partition(index + 1, i1, f, &j);
                    }
                    else if (auto lh = dynamic_cast<const UnilinearFunction<R, D> *>(h)) {
                        UnilinearFunction<R, D> j(i1.getLower(), i1.getUpper(), lh->getValue(i1.getLower()) + cg->getConstantValue(), lh->getValue(i1.getUpper()) + cg->getConstantValue(), lh->getDimension());
                        partition(index + 1, i1, f, &j);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lg = dynamic_cast<const UnilinearFunction<R, D> *>(g)) {
                    if (auto ch = dynamic_cast<const ConstantFunction<R, D> *>(h)) {
                        UnilinearFunction<R, D> j(i1.getLower(), i1.getUpper(), lg->getValue(i1.getLower()) + ch->getConstantValue(), lg->getValue(i1.getUpper()) + ch->getConstantValue(), lg->getDimension());
                        partition(index + 1, i1, f, &j);
                    }
                    else if (auto lh = dynamic_cast<const UnilinearFunction<R, D> *>(h)) {
                        if (lg->getDimension() == lh->getDimension()) {
                            UnilinearFunction<R, D> j(i1.getLower(), i1.getUpper(), lg->getValue(i1.getLower()) + lh->getValue(i1.getLower()), lg->getValue(i1.getUpper()) + lh->getValue(i1.getUpper()), lg->getDimension());
                            partition(index + 1, i1, f, &j);
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
        for (auto f : fs)
            if (!f->isFinite(i))
                return false;
        return true;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Σ ";
        bool first = true;
        for (auto f : fs) {
            if (first)
                first = false;
            else
                os << "\n" << std::string(level + 3, ' ');
            f->printStructure(os, level + 3);
        }
        os << ")";
    }
};

template<typename R, typename X, typename Y, int DIMS, typename RI>
class INET_API IntegratedFunction<R, Domain<X, Y>, DIMS, RI, Domain<X>> : public FunctionBase<RI, Domain<X>>
{
    const Ptr<const IFunction<R, Domain<X, Y>>> f;

  public:
    IntegratedFunction(const Ptr<const IFunction<R, Domain<X, Y>>>& f): f(f) { }

    virtual RI getValue(const Point<X>& p) const override {
        Point<X, Y> l1(std::get<0>(p), getLowerBound<Y>());
        Point<X, Y> u1(std::get<0>(p), getUpperBound<Y>());
        RI ri(0);
        Interval<X, Y> i1(l1, u1, DIMS, DIMS, DIMS);
        f->partition(i1, [&] (const Interval<X, Y>& i2, const IFunction<R, Domain<X, Y>> *g) {
            R r = g->getIntegral(i2);
            ri += RI(toDouble(r));
        });
        return ri;
    }

    virtual void partition(const Interval<X>& i, std::function<void (const Interval<X>&, const IFunction<RI, Domain<X>> *)> g) const override {
        Point<X, Y> l1(std::get<0>(i.getLower()), getLowerBound<Y>());
        Point<X, Y> u1(std::get<0>(i.getUpper()), getUpperBound<Y>());
        Interval<X, Y> i1(l1, u1, (i.getLowerClosed() & 0b1) << 1, (i.getUpperClosed() & 0b1) << 1, (i.getFixed() & 0b1) << 1);
        std::set<X> xs;
        f->partition(i1, [&] (const Interval<X, Y>& i2, const IFunction<R, Domain<X, Y>> *h) {
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
                f->partition(i3, [&] (const Interval<X, Y>& i4, const IFunction<R, Domain<X, Y>> *h) {
                    if (dynamic_cast<const ConstantFunction<R, Domain<X, Y>> *>(h)) {
                        R r = h->getIntegral(i4);
                        ri += RI(toDouble(r));
                    }
                    else if (auto lh = dynamic_cast<const UnilinearFunction<R, Domain<X, Y>> *>(h)) {
                        if (lh->getDimension() == 1) {
                            R r = h->getIntegral(i4);
                            ri += RI(toDouble(r));
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                    else
                        throw cRuntimeError("TODO");
                });
                ConstantFunction<RI, Domain<X>> h(ri);
                Point<X> l5(xLower);
                Point<X> u5(xUpper);
                Interval<X> i5(l5, u5, first ? (i.getLowerClosed() & 0b10) >> 1 : 0b1, 0b0, 0b0);
                g(i5, &h);
            }
            xLower = xUpper;
        }
    }
};

template<typename R, typename D, int DIMS, typename RI, typename DI>
class INET_API IntegratedFunction : public FunctionBase<RI, DI>
{
    const Ptr<const IFunction<R, D>> f;

  public:
    IntegratedFunction(const Ptr<const IFunction<R, D>>& f): f(f) { }

    virtual RI getValue(const typename DI::P& p) const override {
        auto l1 = D::P::getLowerBounds();
        auto u1 = D::P::getUpperBounds();
        p.template copyTo<typename D::P, DIMS>(l1);
        p.template copyTo<typename D::P, DIMS>(u1);
        RI ri(0);
        typename D::I i1(l1, u1, DIMS, DIMS, DIMS);
        f->partition(i1, [&] (const typename D::I& i2, const IFunction<R, D> *g) {
            R r = g->getIntegral(i2);
            ri += RI(toDouble(r));
        });
        return ri;
    }

    virtual void partition(const typename DI::I& i, std::function<void (const typename DI::I&, const IFunction<RI, DI> *)> g) const override {
        throw cRuntimeError("TODO");
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Integrated\n" << std::string(level + 2, ' ');
        f->printStructure(os, level + 2);
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
    const Ptr<const IFunction<R, D>> f;

  public:
    ApproximatedFunction(X lower, X upper, X step, const IInterpolator<X, R> *interpolator, const Ptr<const IFunction<R, D>>& f): lower(lower), upper(upper), step(step), interpolator(interpolator), f(f) { }

    virtual R getValue(const typename D::P& p) const override {
        X x = std::get<DIMENSION>(p);
        if (x < lower)
            return f->getValue(p.template getReplaced<X, DIMENSION>(lower));
        else if (x > upper)
            return f->getValue(p.template getReplaced<X, DIMENSION>(upper));
        else {
            X x1 = lower + step * floor(toDouble(x - lower) / toDouble(step));
            X x2 = std::min(upper, x1 + step);
            R r1 = f->getValue(p.template getReplaced<X, DIMENSION>(x1));
            R r2 = f->getValue(p.template getReplaced<X, DIMENSION>(x2));
            return interpolator->getValue(x1, r1, x2, r2, x);
        }
    }

    virtual void partition(const typename D::I& i, std::function<void (const typename D::I&, const IFunction<R, D> *)> g) const override {
        unsigned char b = 1 << std::tuple_size<typename D::P::type>::value >> 1;
        auto m = (b >> DIMENSION);
        auto fixed = i.getFixed() & m;
        const auto& pl = i.getLower();
        const auto& pu = i.getUpper();
        X xl = std::get<DIMENSION>(pl);
        X xu = std::get<DIMENSION>(pu);
        if (xl < lower) {
            f->partition(i.template getFixed<X, DIMENSION>(lower), [&] (const typename D::I& i1, const IFunction<R, D> *h) {
                if (auto ch = dynamic_cast<const ConstantFunction<R, D> *>(h)) {
                    auto p1 = i1.getLower().template getReplaced<X, DIMENSION>(xl);
                    auto p2 = i1.getUpper().template getReplaced<X, DIMENSION>(std::min(lower, xu));
                    ConstantFunction<R, D> j(ch->getConstantValue());
                    // TODO: review closed & fixed flags
                    typename D::I i2(p1, p2, i.getLowerClosed() | fixed, (i.getUpperClosed() & ~m) | fixed, i.getFixed());
                    g(i2, &j);
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
                f->partition(i.template getFixed<X, DIMENSION>(x1), [&] (const typename D::I& i2, const IFunction<R, D> *h1) {
                    f->partition(i2.template getFixed<X, DIMENSION>(x2), [&] (const typename D::I& i3, const IFunction<R, D> *h2) {
                        if (auto ch1 = dynamic_cast<const ConstantFunction<R, D> *>(h1)) {
                            auto r1 = ch1->getConstantValue();
                            if (auto ch2 = dynamic_cast<const ConstantFunction<R, D> *>(h2)) {
                                auto r2 = ch2->getConstantValue();
                                auto i4 = i3.template getReplaced<X, DIMENSION>(j);
                                if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(interpolator)) {
                                    ConstantFunction<R, D> h(interpolator->getValue(x1, r1, x2, r2, (x1 + x2) / 2));
                                    g(i4, &h);
                                }
                                else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                                    auto p3 = i3.getLower();
                                    auto p4 = i3.getUpper();
                                    auto p5 = p3.template getReplaced<X, DIMENSION>(x1);
                                    auto p6 = p4.template getReplaced<X, DIMENSION>(x2);
                                    UnilinearFunction<R, D> h(p5, p6, r1, r2, DIMENSION);
                                    simplifyAndCall(i4, &h, g);
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
        }
        if (xu > upper) {
            f->partition(i.template getFixed<X, DIMENSION>(upper), [&] (const typename D::I& i1, const IFunction<R, D> *h) {
                if (auto ch = dynamic_cast<const ConstantFunction<R, D> *>(h)) {
                    auto p1 = i1.getLower().template getReplaced<X, DIMENSION>(std::max(upper, xl));
                    auto p2 = i1.getUpper().template getReplaced<X, DIMENSION>(xu);
                    ConstantFunction<R, D> j(ch->getConstantValue());
                    // TODO: review closed & fixed flags
                    typename D::I i2(p1, p2, (i.getLowerClosed() & ~m) | fixed, i.getUpperClosed() | fixed, i.getFixed());
                    g(i2, &j);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return f->isFinite(i);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Approximated\n" << std::string(level + 2, ' ');
        f->printStructure(os, level + 2);
        os << ")";
    }
};

template<typename R, typename X, typename Y>
class INET_API ExtrudedFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<Y>>> f;

  public:
    ExtrudedFunction(const Ptr<const IFunction<R, Domain<Y>>>& f) : f(f) { }

    virtual R getValue(const Point<X, Y>& p) const override {
        return f->getValue(Point<Y>(std::get<1>(p)));
    }

    virtual void partition(const Interval<X, Y>& i, std::function<void (const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> g) const override {
        Interval<Y> i1(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), i.getUpperClosed() & 0b01);
        f->partition(i1, [&] (const Interval<Y>& i2, const IFunction<R, Domain<Y>> *h) {
            Point<X, Y> lower(std::get<0>(i.getLower()), std::get<0>(i2.getLower()));
            Point<X, Y> upper(std::get<0>(i.getUpper()), std::get<0>(i2.getUpper()));
            Interval<X, Y> i3(lower, upper, (i.getUpperClosed() & 0b10) | i2.getUpperClosed());
            if (auto ch = dynamic_cast<const ConstantFunction<R, Domain<Y>> *>(h)) {
                ConstantFunction<R, Domain<X, Y>> j(ch->getConstantValue());
                g(i3, &j);
            }
            else if (auto lh = dynamic_cast<const UnilinearFunction<R, Domain<Y>> *>(h)) {
                Point<X, Y> lower(std::get<0>(i.getLower()), std::get<0>(lh->getLower()));
                Point<X, Y> upper(std::get<0>(i.getUpper()), std::get<0>(lh->getUpper()));
                UnilinearFunction<R, Domain<X, Y>> j(lower, upper, lh->getRLower(), lh->getRUpper(), 1);
                g(i3, &j);
            }
            else
                throw cRuntimeError("TODO");
        });
    }

    virtual bool isFinite(const Interval<X, Y>& i) const override {
        Interval<Y> i1(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())), i.getUpperClosed() & 0b01);
        return f->isFinite(i1);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Extruded\n" << std::string(level + 2, ' ');
        f->printStructure(os, level + 2);
        os << ")";
    }
};

//template<typename R, typename D>
//class INET_API MemoizedFunction : public FunctionBase<R, D>
//{
//  protected:
//    const Ptr<const IFunction<R, D>> f;
//
//  public:
//    MemoizedFunction(const Ptr<const IFunction<R, D>>& f) : f(f) {
//        f->partition(f->getDomain(), [] (const typename D::I& i, const IFunction<R, D> *g) {
//            // TODO: store all interval function pairs in a domain subdivision tree structure
//            throw cRuntimeError("TODO");
//        });
//    }
//
//    virtual R getValue(const typename D::P& p) const override {
//        f->getValue(p);
//    }
//
//    virtual void partition(const typename D::I& i, std::function<void (const typename D::I&, const IFunction<R, D> *)> g) const override {
//        // TODO: search in domain subdivision tree structure
//        throw cRuntimeError("TODO");
//    }
//};

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const IFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> g) {
    g(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const UnilinearFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> g) {
    if (f->getRLower() == f->getRUpper()) {
        ConstantFunction<R, D> h(f->getRLower());
        g(i, &h);
    }
    else
        g(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const BilinearFunction<R, D> *f, const std::function<void (const typename D::I&, const IFunction<R, D> *)> g) {
    if (f->getRLowerLower() == f->getRLowerUpper() && f->getRLowerLower() == f->getRUpperLower() && f->getRLowerLower() == f->getRUpperUpper()) {
        ConstantFunction<R, D> h(f->getRLowerLower());
        g(i, &h);
    }
    // TODO: one dimensional linear functions?
    else
        g(i, f);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_FUNCTIONS_H_

