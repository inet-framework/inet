//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PRIMITIVEFUNCTIONS_H
#define __INET_PRIMITIVEFUNCTIONS_H

#include "inet/common/math/FunctionBase.h"
#include "inet/common/math/Interpolators.h"

namespace inet {

namespace math {

template<typename R, typename D>
class INET_API ConstantFunction : public FunctionBase<R, D>
{
  protected:
    const R value;

  public:
    ConstantFunction(R value) : value(value) {}

    virtual R getConstantValue() const { return value; }

    virtual Interval<R> getRange() const override { return Interval<R>(value, value, 0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override { return value; }

    virtual void partition(const typename D::I& i, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override { return std::isfinite(toDouble(value)); }
    virtual bool isNonZero(const typename D::I& i) const override { return value != R(0); }
    virtual R getMin(const typename D::I& i) const override { return value; }
    virtual R getMax(const typename D::I& i) const override { return value; }
    virtual R getMean(const typename D::I& i) const override { return value; }
    virtual R getIntegral(const typename D::I& i) const override { return value == R(0) ? value : value *i.getVolume(); }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "constant over " << i << "\n" << std::string(level + 2, ' ') << "→ " << value << std::endl;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "Constant " << value;
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
        lower(lower), upper(upper), rLower(rLower), rUpper(rUpper), dimension(dimension) {}

    virtual const typename D::P& getLower() const { return lower; }
    virtual const typename D::P& getUpper() const { return upper; }
    virtual R getRLower() const { return rLower; }
    virtual R getRUpper() const { return rUpper; }
    virtual int getDimension() const { return dimension; }

    virtual double getA() const { return toDouble(rUpper - rLower) / toDouble(upper.get(dimension) - lower.get(dimension)); }
    virtual double getB() const { return (toDouble(rLower) * upper.get(dimension) - toDouble(rUpper) * lower.get(dimension)) / (upper.get(dimension) - lower.get(dimension)); }

    virtual Interval<R> getRange() const override { return Interval<R>(math::minnan(rLower, rUpper), math::maxnan(rLower, rUpper), 0b1, 0b1, 0b0); }

    virtual R getValue(const typename D::P& p) const override {
        double alpha = (p - lower).get(dimension) / (upper - lower).get(dimension);
        return rLower * (1 - alpha) + rUpper * alpha;
    }

    virtual void partition(const typename D::I& i, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return std::isfinite(toDouble(rLower)) && std::isfinite(toDouble(rUpper));
    }

    virtual bool isNonZero(const typename D::I& i) const override {
        return rLower != R(0) || rUpper != R(0);
    }

    virtual R getMin(const typename D::I& i) const override {
        return math::minnan(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const typename D::I& i) const override {
        return math::maxnan(getValue(i.getLower()), getValue(i.getUpper()));
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
        dimension1(dimension1), dimension2(dimension2) {}

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

    virtual Interval<R> getRange() const override {
        return Interval<R>(math::minnan(math::minnan(rLowerLower, rLowerUpper), math::minnan(rUpperLower, rUpperUpper)),
                math::maxnan(math::maxnan(rLowerLower, rLowerUpper), math::maxnan(rUpperLower, rUpperUpper)),
                0b1, 0b1, 0b0);
    }

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

    virtual void partition(const typename D::I& i, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual bool isFinite(const typename D::I& i) const override {
        return std::isfinite(toDouble(rLowerLower)) && std::isfinite(toDouble(rLowerUpper)) && std::isfinite(toDouble(rUpperLower)) && std::isfinite(toDouble(rUpperUpper));
    }

    virtual bool isNonZero(const typename D::I& i) const override {
        return rLowerLower != R(0) || rLowerUpper != R(0) || rUpperLower != R(0) || rUpperUpper != R(0);
    }

    virtual R getMin(const typename D::I& i) const override {
        auto j = getOtherInterval(i);
        return math::minnan(math::minnan(getValue(i.getLower()), getValue(j.getLower())), math::minnan(getValue(j.getUpper()), getValue(i.getUpper())));
    }

    virtual R getMax(const typename D::I& i) const override {
        auto j = getOtherInterval(i);
        return math::maxnan(math::maxnan(getValue(i.getLower()), getValue(j.getLower())), math::maxnan(getValue(j.getUpper()), getValue(i.getUpper())));
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
    UnireciprocalFunction(double a, double b, double c, double d, int dimension) : a(a), b(b), c(c), d(d), dimension(dimension) {}

    virtual int getDimension() const { return dimension; }

    virtual R getValue(const typename D::P& p) const override {
        double x = p.get(dimension);
        return R(a * x + b) / (c * x + d);
    }

    virtual void partition(const typename D::I& i, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) const override {
        callback(i, this);
    }

    virtual R getMin(const typename D::I& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getLowerBound<R>();
        else
            return math::minnan(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const typename D::I& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getUpperBound<R>();
        else
            return math::maxnan(getValue(i.getLower()), getValue(i.getUpper()));
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
//        a0(a0), a1(a1), a2(a2), a3(a3), b0(b0), b1(b1), b2(b2), b3(b3), dimension1(dimension1), dimension2(dimension2) {}
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

//template<typename R, typename D>
//class INET_API QuadraticFunction : public Function<R, D>
//{
//  protected:
//    const double a;
//    const double b;
//    const double c;
//
//  public:
//    QuadraticFunction(double a, double b, double c) : a(a), b(b), c(c) {}
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
 * Some constant value r between lower and upper and zero otherwise.
 */
template<typename R, typename X>
class INET_API Boxcar1DFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X lower;
    const X upper;
    const R value;

  public:
    Boxcar1DFunction(X lower, X upper, R value) : lower(lower), upper(upper), value(value) {
        ASSERT(value > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), value, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X>& p) const override {
        return std::get<0>(p) < lower || std::get<0>(p) >= upper ? R(0) : value;
    }

    virtual void partition(const Interval<X>& i, const std::function<void(const Interval<X>&, const IFunction<R, Domain<X>> *)> callback) const override {
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
    virtual bool isNonZero(const Interval<X>& i) const override { return value != R(0) && lower <= std::get<0>(i.getLower()) && std::get<0>(i.getUpper()) <= upper; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Boxcar1D, [" << lower << " … " << upper << "] → " << value << ")";
    }
};

/**
 * Some constant value r between (lowerX, lowerY) and (upperX, upperY) and zero otherwise.
 */
template<typename R, typename X, typename Y>
class INET_API Boxcar2DFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const X lowerX;
    const X upperX;
    const Y lowerY;
    const Y upperY;
    const R value;

  protected:
    void call(const Interval<X, Y>& i, const std::function<void(const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback, R r) const {
        if (!i.isEmpty()) {
            ConstantFunction<R, Domain<X, Y>> g(r);
            callback(i, &g);
        }
    }

  public:
    Boxcar2DFunction(X lowerX, X upperX, Y lowerY, Y upperY, R value) :
        lowerX(lowerX), upperX(upperX), lowerY(lowerY), upperY(upperY), value(value)
    {
        ASSERT(value > R(0));
    }

    virtual Interval<R> getRange() const override { return Interval<R>(R(0), value, 0b1, 0b1, 0b0); }

    virtual R getValue(const Point<X, Y>& p) const override {
        return std::get<0>(p) < lowerX || std::get<0>(p) >= upperX || std::get<1>(p) < lowerY || std::get<1>(p) >= upperY ? R(0) : value;
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void(const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
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
    virtual bool isNonZero(const Interval<X, Y>& i) const override {
        return value != R(0) &&
               lowerX <= std::get<0>(i.getLower()) && std::get<0>(i.getUpper()) <= upperX &&
               lowerY <= std::get<1>(i.getLower()) && std::get<1>(i.getUpper()) <= upperY;
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(Boxcar2D, [" << lowerX << " … " << upperX << "] x [" << lowerY << " … " << upperY << "] → " << value << ")";
    }
};

/**
 * The one-dimensional Gauss function.
 */
template<typename R, typename X>
class INET_API GaussFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X mean;
    const X stddev;

  public:
    GaussFunction(X mean, X stddev) : mean(mean), stddev(stddev) {}

    virtual R getValue(const Point<X>& p) const override {
        static const double c = 1 / sqrt(2 * M_PI);
        X x = std::get<0>(p);
        double a = toDouble((x - mean) / stddev);
        return R(c / toDouble(stddev) * std::exp(-0.5 * a * a));
    }
};

/**
 * One-dimensional periodic function with a sawtooth shape.
 */
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
    {}

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

/**
 * One-dimensional interpolated (e.g. constant, linear) function between intervals defined by points positioned periodically on the X axis.
 */
template<typename R, typename X>
class INET_API PeriodicallyInterpolated1DFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const X start;
    const X end;
    const X step;
    const IInterpolator<X, R>& interpolator;
    const std::vector<R> rs;

  public:
    PeriodicallyInterpolated1DFunction(X start, X end, const IInterpolator<X, R>& interpolator, const std::vector<R>& rs) :
        start(start), end(end), step((end - start) / (rs.size() - 1)), interpolator(interpolator), rs(rs) {}

    virtual R getValue(const Point<X>& p) const override {
        X x = std::get<0>(p);
        int index = std::floor(toDouble(x - start) / toDouble(step));
        if (index < 0 || index > rs.size() - 2)
            return R(0);
        else {
            R r1 = rs[index];
            R r2 = rs[index + 1];
            X x1 = start + step * index;
            X x2 = x1 + step;
            return interpolator.getValue(x1, r1, x2, r2, x);
        }
    }

    virtual void partition(const Interval<X>& i, const std::function<void(const Interval<X>&, const IFunction<R, Domain<X>> *)> callback) const override {
        const auto& i1 = i.getIntersected(Interval<X>(getLowerBound<X>(), Point<X>(start), 0b0, 0b0, 0b0));
        if (!i1.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            callback(i1, &g);
        }
        const auto& i2 = i.getIntersected(Interval<X>(Point<X>(start), Point<X>(end), 0b1, 0b0, 0b0));
        if (!i2.isEmpty()) {
            int startIndex = std::max(0, (int)std::floor(toDouble(std::get<0>(i.getLower()) - start) / toDouble(step)));
            int endIndex = std::min((int)rs.size() - 1, (int)std::ceil(toDouble(std::get<0>(i.getUpper()) - start) / toDouble(step)));
            for (int index = startIndex; index < endIndex; index++) {
                Point<X> startPoint(start + step * index);
                Point<X> endPoint(start + step * (index + 1));
                const auto& i3 = i.getIntersected(Interval<X>(startPoint, endPoint, 0b1, 0b0, 0b0));
                if (!i3.isEmpty()) {
                    if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(&interpolator)) {
                        R r = getValue((startPoint + endPoint) / 2);
                        ConstantFunction<R, Domain<X>> g(r);
                        callback(i3, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
            }
        }
        const auto& i4 = i.getIntersected(Interval<X>(Point<X>(end), getUpperBound<X>(), 0b1, 0b0, 0b0));
        if (!i4.isEmpty()) {
            ConstantFunction<R, Domain<X>> g(R(0));
            callback(i4, &g);
        }
    }
};

template<typename R, typename X, typename Y>
class INET_API PeriodicallyInterpolated2DFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const X startX;
    const X endX;
    const X stepX;
    int sizeX;
    const Y startY;
    const Y endY;
    const Y stepY;
    int sizeY;
    const IInterpolator<X, R>& interpolatorX;
    const IInterpolator<Y, R>& interpolatorY;
    const std::vector<R> rs;

  protected:
    virtual R getValueInterpolatedAlongX(X x, int indexX, int indexY) const {
        R r1 = rs[sizeX * indexY + indexX];
        R r2 = rs[sizeX * indexY + indexX + 1];
        X x1 = startX + stepX * indexX;
        X x2 = x1 + stepX;
        return interpolatorX.getValue(x1, r1, x2, r2, x);
    }

    void call(const Interval<X, Y>& i, const std::function<void(const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const {
        if (!i.isEmpty()) {
            ConstantFunction<R, Domain<X, Y>> g(R(0));
            callback(i, &g);
        }
    }

  public:
    PeriodicallyInterpolated2DFunction(X startX, X endX, int sizeX, Y startY, Y endY, int sizeY, const IInterpolator<X, R>& interpolatorX, const IInterpolator<Y, R>& interpolatorY, const std::vector<R>& rs) :
        startX(startX), endX(endX), stepX((endX - startX) / (sizeX - 1)), sizeX(sizeX),
        startY(startY), endY(endY), stepY((endY - startY) / (sizeY - 1)), sizeY(sizeY),
        interpolatorX(interpolatorX), interpolatorY(interpolatorY), rs(rs)
    {
        ASSERT(rs.size() == sizeX * sizeY);
    }

    virtual R getValue(const Point<X, Y>& p) const override {
        X x = std::get<0>(p);
        Y y = std::get<1>(p);
        int indexX = std::floor(toDouble(x - startX) / toDouble(stepX));
        int indexY = std::floor(toDouble(y - startY) / toDouble(stepY));
        if (indexX < 0 || indexX > sizeX - 2 || indexY < 0 || indexY > sizeY - 2)
            return R(0);
        else {
            R r1 = getValueInterpolatedAlongX(x, indexX, indexY);
            R r2 = getValueInterpolatedAlongX(x, indexX, indexY + 1);
            Y y1 = startY + stepY * indexY;
            Y y2 = y1 + stepY;
            return interpolatorY.getValue(y1, r1, y2, r2, y);
        }
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void(const Interval<X, Y>&, const IFunction<R, Domain<X, Y>> *)> callback) const override {
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), getLowerBound<Y>()), Point<X, Y>(X(startX), Y(startY)), 0b00, 0b00, 0b00)), callback);
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(startX), getLowerBound<Y>()), Point<X, Y>(X(endX), Y(startY)), 0b10, 0b00, 0b00)), callback);
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(endX), getLowerBound<Y>()), Point<X, Y>(getUpperBound<X>(), Y(startY)), 0b10, 0b00, 0b00)), callback);

        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(startY)), Point<X, Y>(X(startX), Y(endY)), 0b01, 0b00, 0b00)), callback);
        const auto& i1 = i.getIntersected(Interval<X, Y>(Point<X, Y>(X(startX), Y(startY)), Point<X, Y>(X(endX), Y(endY)), 0b11, 0b00, 0b00));
        if (!i1.isEmpty()) {
            int startIndexX = std::max(0, (int)std::floor(toDouble(std::get<0>(i.getLower()) - startX) / toDouble(stepX)));
            int endIndexX = std::min(sizeX - 1, (int)std::ceil(toDouble(std::get<0>(i.getUpper()) - startX) / toDouble(stepX)));
            for (int indexX = startIndexX; indexX < endIndexX; indexX++) {
                int startIndexY = std::max(0, (int)std::floor(toDouble(std::get<1>(i.getLower()) - startY) / toDouble(stepY)));
                int endIndexY = std::min(sizeY - 1, (int)std::ceil(toDouble(std::get<1>(i.getUpper()) - startY) / toDouble(stepY)));
                for (int indexY = startIndexY; indexY < endIndexY; indexY++) {
                    Point<X, Y> startPoint(startX + stepX * indexX, startY + stepY * indexY);
                    Point<X, Y> endPoint(startX + stepX * (indexX + 1), startY + stepY * (indexY + 1));
                    const auto& i3 = i.getIntersected(Interval<X, Y>(startPoint, endPoint, 0b11, 0b00, 0b00));
                    if (!i3.isEmpty()) {
                        if (dynamic_cast<const ConstantInterpolatorBase<X, R> *>(&interpolatorX) && dynamic_cast<const ConstantInterpolatorBase<Y, R> *>(&interpolatorY)) {
                            R r = getValue((startPoint + endPoint) / 2);
                            ConstantFunction<R, Domain<X, Y>> g(r);
                            callback(i3, &g);
                        }
                        else
                            throw cRuntimeError("TODO");
                    }
                }
            }
        }
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(endX), Y(startY)), Point<X, Y>(getUpperBound<X>(), Y(endY)), 0b11, 0b00, 0b00)), callback);

        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(getLowerBound<X>(), Y(endY)), Point<X, Y>(X(startX), getUpperBound<Y>()), 0b01, 0b00, 0b00)), callback);
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(startX), Y(endY)), Point<X, Y>(X(endX), getUpperBound<Y>()), 0b11, 0b00, 0b00)), callback);
        call(i.getIntersected(Interval<X, Y>(Point<X, Y>(X(endX), Y(endY)), Point<X, Y>(getUpperBound<X>(), getUpperBound<Y>()), 0b11, 0b00, 0b00)), callback);
    }
};

/**
 * One-dimensional interpolated (e.g. constant, linear) function between intervals defined by points on the X axis.
 */
template<typename R, typename X>
class INET_API Interpolated1DFunction : public FunctionBase<R, Domain<X>>
{
  protected:
    const std::map<X, std::pair<R, const IInterpolator<X, R> *>> rs;

  public:
    Interpolated1DFunction(const std::map<X, R>& rs, const IInterpolator<X, R> *interpolator) : rs([&] () {
        std::map<X, std::pair<R, const IInterpolator<X, R> *>> result;
        for (auto it : rs)
            result[it.first] = { it.second, interpolator };
        return result;
    } ()) {}

    Interpolated1DFunction(const std::map<X, std::pair<R, const IInterpolator<X, R> *>>& rs) : rs(rs) {}

    virtual R getValue(const Point<X>& p) const override {
        X x = std::get<0>(p);
        auto it = rs.equal_range(x);
        auto& lt = it.first;
        auto& ut = it.second;
        // TODO this nested if looks horrible
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

    virtual void partition(const Interval<X>& i, const std::function<void(const Interval<X>&, const IFunction<R, Domain<X>> *)> callback) const override {
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
        os << "(Interpolated1D";
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

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const IFunction<R, D> *f, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) {
    callback(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const UnilinearFunction<R, D> *f, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) {
    if (f->getRLower() == f->getRUpper()) {
        ConstantFunction<R, D> g(f->getRLower());
        callback(i, &g);
    }
    else
        callback(i, f);
}

template<typename R, typename D>
void simplifyAndCall(const typename D::I& i, const BilinearFunction<R, D> *f, const std::function<void(const typename D::I&, const IFunction<R, D> *)> callback) {
    if (f->getRLowerLower() == f->getRLowerUpper() && f->getRLowerLower() == f->getRUpperLower() && f->getRLowerLower() == f->getRUpperUpper()) {
        ConstantFunction<R, D> g(f->getRLowerLower());
        callback(i, &g);
    }
    // TODO simplify to one dimensional linear functions?
    else
        callback(i, f);
}

} // namespace math

} // namespace inet

#endif

