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

#ifndef __INET_MATH_COMPOUNDFUNCTIONS_H_
#define __INET_MATH_COMPOUNDFUNCTIONS_H_

#include "inet/common/math/PrimitiveFunctions.h"

namespace inet {

namespace math {

/**
 * Limits the domain of a multidimensional function.
 */
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

/**
 * Shifts the domain of a multidimensional function.
 */
template<typename R, typename D>
class INET_API DomainShiftedFunction : public FunctionBase<R, D>
{
  protected:
    const Ptr<const IFunction<R, D>> function;
    const typename D::P shift;

  public:
    DomainShiftedFunction(const Ptr<const IFunction<R, D>>& f, const typename D::P& s) : function(f), shift(s) { }

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
        os << "(DomainShifted, shift = " << shift << "\n" << std::string(level + 2, ' ');
        function->printStructure(os, level + 2);
        os << ")";
    }
};

/**
 * Truncates the values of a multidimensional function outside the given domain to 0.
 */
//template<typename R, typename D>
//class INET_API ValueTruncatedFunction : public FunctionBase<R, D>
//{
//  protected:
//    const Ptr<const IFunction<R, D>> function;
//    const Interval<R> range;
//    const typename D::I domain;
//
//  public:
//    ValueTruncatedFunction(const Ptr<const IFunction<R, D>>& function, const typename D::I& domain) :
//        function(function), range(Interval<R>(std::min(R(0), function->getMin(domain)), std::max(R(0), function->getMax(domain)), 0b1, 0b1, 0b0)), domain(domain)
//    { }
//
//    virtual Interval<R> getRange() const override { return range; }
//    virtual typename D::I getDomain() const override { return domain; }
//
//    virtual R getValue(const typename D::P& p) const override {
//        if (domain.contains(p))
//            return function->getValue(p);
//        else
//            return R(0);
//    }
//
//};

/**
 * Caches the values of a multidimensional function.
 */
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
#ifdef _OPENMP
#pragma omp critical
#endif
            {
                cache[p] = v;
                if ((int)cache.size() > limit)
                    cache.clear();
            }
            return v;
        }
    }

    virtual void partition(const typename D::I& i, std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        function->partition(i, callback);
    }
};

/**
 * Combines 2 one-dimensional functions into a two-dimensional function.
 */
template<typename R, typename X, typename Y>
class INET_API Combined2DFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<X>>> functionX;
    const Ptr<const IFunction<double, Domain<Y>>> functionY;
    // TODO: std::binary_function<X, Y> operation;

  public:
    Combined2DFunction(const Ptr<const IFunction<R, Domain<X>>>& functionX, const Ptr<const IFunction<double, Domain<Y>>>& functionY) :
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
 * Modulates the domain of a two-dimensional function with the values of a one-dimensional function.
 */
template<typename R, typename X, typename Y>
class INET_API DomainModulated2DFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<X, Y>>> function;
    const Ptr<const IFunction<Y, Domain<X>>> modulator;
    // TODO: std::binary_function<Y, Y> operation;

  public:
    DomainModulated2DFunction(const Ptr<const IFunction<R, Domain<X, Y>>>& function, const Ptr<const IFunction<Y, Domain<X>>>& modulator) :
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

/**
 * Approximates the values and partitioning of a multidimensional function along one of the dimensions.
 */
template<typename R, typename D, int DIMENSION, typename X>
class INET_API ApproximatedFunction : public FunctionBase<R, D>
{
  protected:
    // TODO: add an ISampler (e.g lower ... upper by step) interface and refactor to be based on that
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

/**
 * Extrudes a one-dimensional function into a two-dimensional function.
 */
template<typename R, typename X, typename Y>
class INET_API Extruded2DFunction : public FunctionBase<R, Domain<X, Y>>
{
  protected:
    const Ptr<const IFunction<R, Domain<Y>>> function;

  public:
    Extruded2DFunction(const Ptr<const IFunction<R, Domain<Y>>>& function) : function(function) { }

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

template<typename R, typename D, int DIMS, typename RI, typename DI>
class INET_API IntegratedFunction;

/**
 * Integrates a multidimensional function in one of the dimensions producing a
 * function with one less number of dimensions.
 */
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

template<typename R, typename D, int DIMS, typename RI, typename DI>
Ptr<const IFunction<RI, DI>> integrate(const Ptr<const IFunction<R, D>>& f) {
    return makeShared<IntegratedFunction<R, D, DIMS, RI, DI>>(f);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_COMPOUNDFUNCTIONS_H_

