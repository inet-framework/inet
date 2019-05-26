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

template<typename R, typename ... DS>
class INET_API DomainLimitedFunction;

template<typename R, typename ... DS>
class INET_API AdditionFunction;

template<typename R, typename ... DS>
class INET_API SubtractionFunction;

template<typename R, typename ... DS>
class INET_API MultiplicationFunction;

template<typename R, typename ... DS>
class INET_API DivisionFunction;

template<typename R, typename ... DS>
class INET_API FunctionBase : public IFunction<R, DS ...>
{
  public:
    virtual Interval<R> getRange() const {
        return Interval<R>(getLowerBoundary<R>(), getUpperBoundary<R>());
    }

    virtual Interval<DS ...> getDomain() const {
        return Interval<DS ...>(getLowerBoundaries<DS ...>(), getUpperBoundaries<DS ...>());
    }

    virtual Ptr<const IFunction<R, DS ...>> limitDomain(const Interval<DS ...>& i) const {
        return makeShared<DomainLimitedFunction<R, DS ...>>(const_cast<FunctionBase<R, DS ...> *>(this)->shared_from_this(), i.intersect(getDomain()));
    }

    virtual R getMin() const { return getMin(getDomain()); }
    virtual R getMin(const Interval<DS ...>& i) const {
        R result(getUpperBoundary<R>());
        this->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *f) {
            result = std::min(f->getMin(i1), result);
        });
        return result;
    }

    virtual R getMax() const { return getMax(getDomain()); }
    virtual R getMax(const Interval<DS ...>& i) const {
        R result(getLowerBoundary<R>());
        this->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *f) {
            result = std::max(f->getMax(i1), result);
        });
        return result;
    }

    virtual R getMean() const { return getMean(getDomain()); }
    virtual R getMean(const Interval<DS ...>& i) const {
        double totalVolume = 0;
        R result(0);
        this->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *f) {
            auto volume = i1.getVolume(i);
            totalVolume += volume;
            R value = f->getMean(i1);
            if (value != R(0))
                result += volume * value;
        });
        return result / totalVolume;
    }

    virtual const Ptr<const IFunction<R, DS ...>> add(const Ptr<const IFunction<R, DS ...>>& o) const override {
        return makeShared<AdditionFunction<R, DS ...>>(const_cast<FunctionBase<R, DS ...> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, DS ...>> subtract(const Ptr<const IFunction<R, DS ...>>& o) const override {
        return makeShared<SubtractionFunction<R, DS ...>>(const_cast<FunctionBase<R, DS ...> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, DS ...>> multiply(const Ptr<const IFunction<double, DS ...>>& o) const override {
        return makeShared<MultiplicationFunction<R, DS ...>>(const_cast<FunctionBase<R, DS ...> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<double, DS ...>> divide(const Ptr<const IFunction<R, DS ...>>& o) const override {
        return makeShared<DivisionFunction<R, DS ...>>(const_cast<FunctionBase<R, DS ...> *>(this)->shared_from_this(), o);
    }
};

template<typename R, typename ... DS>
inline std::ostream& operator<<(std::ostream& os, const IFunction<R, DS ...>& f)
{
    os << "f {" << std::endl;
    f.partition(f.getDomain(), [&] (const Interval<DS ...>& i, const IFunction<R, DS ...> *g) {
        os << "  i " << i << " -> { ";
        iterateBoundaries<DS ...>(i, std::function<void (const Point<DS ...>&)>([&] (const Point<DS ...>& p) {
            os << "@" << p << " = " << f.getValue(p) << ", ";
        }));
        os << "min = " << g->getMin(i) << ", max = " << g->getMax(i) << ", mean = " << g->getMean(i) << " }" << std::endl;
    });
    return os << "} min = " << f.getMin() << ", max = " << f.getMax() << ", mean = " << f.getMean();
}

template<typename R, typename ... DS>
class INET_API DomainLimitedFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f;
    const Interval<DS ...> domain;

  public:
    DomainLimitedFunction(const Ptr<const IFunction<R, DS ...>>& f, const Interval<DS ...>& domain) : f(f), domain(domain) { }

    virtual Interval<R> getRange() const override { return Interval<R>(this->getMin(domain), this->getMax(domain)); };
    virtual Interval<DS ...> getDomain() const override { return domain; };

    virtual R getValue(const Point<DS ...>& p) const override {
        ASSERT(domain.contains(p));
        return f->getValue(p);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> g) const override {
        auto i1 = i.intersect(domain);
        if (isValidInterval(i1))
            f->partition(i1, g);
    }
};

template<typename R, typename ... DS>
class INET_API ConstantFunction : public FunctionBase<R, DS ...>
{
  protected:
    const R r;

  public:
    ConstantFunction(R r) : r(r) { }

    virtual R getConstantValue() const { return r; }

    virtual Interval<R> getRange() const { return Interval<R>(r, r); }

    virtual R getValue(const Point<DS ...>& p) const override { return r; }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f(i, this);
    }

    virtual R getMin(const Interval<DS ...>& i) const override { return r; }
    virtual R getMax(const Interval<DS ...>& i) const override { return r; }
    virtual R getMean(const Interval<DS ...>& i) const override { return r; }
};

template<typename R, typename X>
class INET_API OneDimensionalBoxcarFunction : public FunctionBase<R, X>
{
  protected:
    const X lower;
    const X upper;
    const R r;

  public:
    OneDimensionalBoxcarFunction(X lower, X upper, R r) : lower(lower), upper(upper), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const { return Interval<R>(R(0), r); }

    virtual R getValue(const Point<X>& p) const override {
        return std::get<0>(p) < lower || std::get<0>(p) >= upper ? R(0) : r;
    }

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, X> *)> f) const override {
        auto i1 = i.intersect(Interval<X>(getLowerBoundary<X>(), Point<X>(lower)));
        if (isValidInterval(i1)) {
            ConstantFunction<R, X> g(R(0));
            f(i1, &g);
        }
        auto i2 = i.intersect(Interval<X>(Point<X>(lower), Point<X>(upper)));
        if (isValidInterval(i2)) {
            ConstantFunction<R, X> g(r);
            f(i2, &g);
        }
        auto i3 = i.intersect(Interval<X>(Point<X>(upper), getUpperBoundary<X>()));
        if (isValidInterval(i3)) {
            ConstantFunction<R, X> g(R(0));
            f(i3, &g);
        }
    }
};

template<typename R, typename X, typename Y>
class INET_API TwoDimensionalBoxcarFunction : public FunctionBase<R, X, Y>
{
  protected:
    const X lowerX;
    const X upperX;
    const Y lowerY;
    const Y upperY;
    const R r;

  protected:
    void callf(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, X, Y> *)> f, R r) const {
        if (isValidInterval(i)) {
            ConstantFunction<R, X, Y> g(r);
            f(i, &g);
        }
    }

  public:
    TwoDimensionalBoxcarFunction(X lowerX, X upperX, Y lowerY, Y upperY, R r) : lowerX(lowerX), upperX(upperX), lowerY(lowerY), upperY(upperY), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const { return Interval<R>(R(0), r); }

    virtual R getValue(const Point<X, Y>& p) const override {
        return std::get<0>(p) < lowerX || std::get<0>(p) >= upperX || std::get<1>(p) < lowerY || std::get<1>(p) >= upperY ? R(0) : r;
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, X, Y> *)> f) const override {
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), getLowerBoundary<Y>()), Point<X, Y>(X(lowerX), Y(lowerY)))), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), getLowerBoundary<Y>()), Point<X, Y>(X(upperX), Y(lowerY)))), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), getLowerBoundary<Y>()), Point<X, Y>(getUpperBoundary<X>(), Y(lowerY)))), f, R(0));

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), Y(lowerY)), Point<X, Y>(X(lowerX), Y(upperY)))), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(lowerY)), Point<X, Y>(X(upperX), Y(upperY)))), f, r);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(lowerY)), Point<X, Y>(getUpperBoundary<X>(), Y(upperY)))), f, R(0));

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), Y(upperY)), Point<X, Y>(X(lowerX), getUpperBoundary<Y>()))), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(upperY)), Point<X, Y>(X(upperX), getUpperBoundary<Y>()))), f, R(0));
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(upperY)), Point<X, Y>(getUpperBoundary<X>(), getUpperBoundary<Y>()))), f, R(0));
    }
};

template<typename R, typename ... DS>
class INET_API LinearInterpolatedFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Point<DS ...> lower; // value is ignored in all but one dimension
    const Point<DS ...> upper; // value is ignored in all but one dimension
    const R rLower;
    const R rUpper;
    const int dimension;

  public:
    LinearInterpolatedFunction(Point<DS ...> lower, Point<DS ...> upper, R rLower, R rUpper, int dimension) : lower(lower), upper(upper), rLower(rLower), rUpper(rUpper), dimension(dimension) { }

    virtual const Point<DS ...>& getLower() const { return lower; }
    virtual const Point<DS ...>& getUpper() const { return upper; }
    virtual R getRLower() const { return rLower; }
    virtual R getRUpper() const { return rUpper; }
    virtual int getDimension() const { return dimension; }

    virtual double getA() const { return toDouble(rUpper - rLower) / toDouble(upper.get(dimension) - lower.get(dimension)); }
    virtual double getB() const { return (toDouble(rLower) * upper.get(dimension) - toDouble(rUpper) * lower.get(dimension)) / (upper.get(dimension) - lower.get(dimension)); }

    virtual Interval<R> getRange() const { return Interval<R>(std::min(rLower, rUpper), std::max(rLower, rUpper)); }
    virtual Interval<DS ...> getDomain() const { return Interval<DS ...>(lower, upper); };

    virtual R getValue(const Point<DS ...>& p) const override {
        double alpha = (p - lower).get(dimension) / (upper - lower).get(dimension);
        return rLower * (1 - alpha) + rUpper * alpha;
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f(i, this);
    }

    virtual R getMin(const Interval<DS ...>& i) const override {
        return std::min(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const Interval<DS ...>& i) const override {
        return std::max(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMean(const Interval<DS ...>& i) const override {
        return getValue((i.getLower() + i.getUpper()) / 2);
    }
};

template<typename R, typename X>
class INET_API OneDimensionalInterpolatedFunction : public FunctionBase<R, X>
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
        if (lt != rs.end() && lt->first == x)
            return lt->second.first;
        else {
            ASSERT(lt != rs.end() && ut != rs.end());
            lt--;
            const auto interpolator = lt->second.second;
            return interpolator->getValue(lt->first, lt->second.first, ut->first, ut->second.first, x);
        }
    }

    virtual void partition(const Interval<X>& i, const std::function<void (const Interval<X>&, const IFunction<R, X> *)> f) const override {
        auto lt = rs.equal_range(std::get<0>(i.getLower()));
        auto ut = rs.equal_range(std::get<0>(i.getUpper()));
        auto it = lt.first;
        for (auto jt = lt.first; jt != ut.second; jt++) {
            if (it != jt) {
                auto i1 = i.intersect(Interval<X>(Point<X>(it->first), Point<X>(jt->first)));
                if (isValidInterval(i1)) {
                    const auto interpolator = it->second.second;
                    if (dynamic_cast<const EitherInterpolator<X, R> *>(interpolator)) {
                        ConstantFunction<R, X> g(it->second.first);
                        f(i1, &g);

                    }
                    else if (dynamic_cast<const SmallerInterpolator<X, R> *>(interpolator)) {
                        ConstantFunction<R, X> g(it->second.first); // TODO: what about the ends?
                        f(i1, &g);
                    }
                    else if (dynamic_cast<const GreaterInterpolator<X, R> *>(interpolator)) {
                        ConstantFunction<R, X> g(jt->second.first); // TODO: what about the ends?
                        f(i1, &g);
                    }
                    else if (dynamic_cast<const LinearInterpolator<X, R> *>(interpolator)) {
                        LinearInterpolatedFunction<R, X> g(Point<X>(it->first), Point<X>(jt->first), it->second.first, jt->second.first, 0);
                        f(i1, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
            }
            it = jt;
        }
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
//    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, X, Y> *)> f) const override {
//        throw cRuntimeError("TODO");
//    }
//};

//template<typename R, typename D0, typename ... DS>
//class INET_API FunctionInterpolatingFunction : public Function<R, DS ...>
//{
//  protected:
//    const IInterpolator<R, D0>& i;
//    const std::map<D0, const IFunction<R, DS ...> *> fs;
//
//  public:
//    FunctionInterpolatingFunction(const IInterpolator<R, D0>& i, const std::map<D0, const IFunction<R, DS ...> *>& fs) : i(i), fs(fs) { }
//
//    virtual R getValue(const Point<D0, DS ...>& p) const override {
//        D0 x = std::get<0>(p);
//        auto lt = fs.lower_bound(x);
//        auto ut = fs.upper_bound(x);
//        ASSERT(lt != fs.end() && ut != fs.end());
//        Point<DS ...> q;
//        return i.get(lt->first, lt->second->getValue(q), ut->first, ut->second->getValue(q), x);
//    }
//
//    virtual void partition(const Interval<D0, DS ...>& i, const std::function<void (const Interval<D0, DS ...>&, const IFunction<R, DS ...> *)> f) const override {
//        throw cRuntimeError("TODO");
//    }
//};

//template<typename R, typename ... DS>
//class INET_API GaussFunction : public Function<R, DS ...>
//{
//  protected:
//    const R mean;
//    const R stddev;
//
//  public:
//    GaussFunction(R mean, R stddev) : mean(mean), stddev(stddev) { }
//
//    virtual R getValue(const Point<DS ...>& p) const override {
//        throw cRuntimeError("TODO");
//    }
//
//    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
//        throw cRuntimeError("TODO");
//    }
//};

template<typename R, typename X, typename Y>
class INET_API OrthogonalCombinatorFunction : public FunctionBase<R, X, Y>
{
  protected:
    const Ptr<const IFunction<R, X>> f;
    const Ptr<const IFunction<double, Y>> g;

  public:
    OrthogonalCombinatorFunction(const Ptr<const IFunction<R, X>>& f, const Ptr<const IFunction<double, Y>>& g) : f(f), g(g) { }

    virtual R getValue(const Point<X, Y>& p) const override {
        return f->getValue(std::get<0>(p)) * g->getValue(std::get<1>(p));
    }

    virtual void partition(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>&, const IFunction<R, X, Y> *)> h) const override {
        Interval<X> ix(Point<X>(std::get<0>(i.getLower())), Point<X>(std::get<0>(i.getUpper())));
        Interval<Y> iy(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())));
        f->partition(ix, [&] (const Interval<X>& ixf, const IFunction<R, X> *if1) {
            g->partition(iy, [&] (const Interval<Y>& iyg, const IFunction<double, Y> *if2) {
                Point<X, Y> lower(std::get<0>(ixf.getLower()), std::get<0>(iyg.getLower()));
                Point<X, Y> upper(std::get<0>(ixf.getUpper()), std::get<0>(iyg.getUpper()));
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, X> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, Y> *>(if2)) {
                        ConstantFunction<R, X, Y> g(cif1->getConstantValue() * cif2->getConstantValue());
                        h(Interval<X, Y>(lower, upper), &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<double, Y> *>(if2)) {
                        LinearInterpolatedFunction<R, X, Y> g(lower, upper, lif2->getValue(iyg.getLower()) * cif1->getConstantValue(), lif2->getValue(iyg.getUpper()) * cif1->getConstantValue(), 1);
                        h(Interval<X, Y>(lower, upper), &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const LinearInterpolatedFunction<R, X> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, Y> *>(if2)) {
                        LinearInterpolatedFunction<R, X, Y> g(lower, upper, lif1->getValue(ixf.getLower()) * cif2->getConstantValue(), lif1->getValue(ixf.getUpper()) * cif2->getConstantValue(), 0);
                        h(Interval<X, Y>(lower, upper), &g);
                    }
                    else {
                        // QuadraticFunction<double, X, Y> g(); // TODO:
                        throw cRuntimeError("TODO");
                    }
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API ShiftFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f;
    const Point<DS ...> s;

  public:
    ShiftFunction(const Ptr<const IFunction<R, DS ...>>& f, const Point<DS ...>& s) : f(f), s(s) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f->getValue(p - s);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> g) const override {
        f->partition(Interval<DS ...>(i.getLower() - s, i.getUpper() - s), [&] (const Interval<DS ...>& j, const IFunction<R, DS ...> *jf) {
            if (auto cjf = dynamic_cast<const ConstantFunction<R, DS ...> *>(jf))
                g(Interval<DS ...>(j.getLower() + s, j.getUpper() + s), jf);
            else if (auto ljf = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(jf)) {
                LinearInterpolatedFunction<R, DS ...> h(j.getLower() + s, j.getUpper() + s, ljf->getValue(j.getLower()), ljf->getValue(j.getUpper()), ljf->getDimension());
                g(Interval<DS ...>(j.getLower() + s, j.getUpper() + s), &h);
            }
            else {
                ShiftFunction h(const_cast<IFunction<R, DS ...> *>(jf)->shared_from_this(), s);
                g(Interval<DS ...>(j.getLower() + s, j.getUpper() + s), &h);
            }
        });
    }
};

// TODO:
//template<typename R, typename ... DS>
//class INET_API QuadraticFunction : public Function<R, DS ...>
//{
//  protected:
//    const double a;
//    const double b;
//    const double c;
//
//  public:
//    QuadraticFunction(double a, double b, double c) : a(a), b(b), c(c) { }
//
//    virtual R getValue(const Point<DS ...>& p) const override {
//        return R(0);
//    }
//
//    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
//        throw cRuntimeError("TODO");
//    }
//};

template<typename R, typename ... DS>
class INET_API ReciprocalFunction : public FunctionBase<R, DS ...>
{
  protected:
    // f(x) = (a * x + b) / (c * x + d)
    const double a;
    const double b;
    const double c;
    const double d;
    const int dimension;

  protected:
    double getIntegral(const Point<DS ...>& p) const {
        // https://www.wolframalpha.com/input/?i=integrate+(a+*+x+%2B+b)+%2F+(c+*+x+%2B+d)
        double x = p.get(dimension);
        return (a * c * x + (b * c - a * d) * std::log(d + c * x)) / (c * c);
    }

  public:
    ReciprocalFunction(double a, double b, double c, double d, int dimension) : a(a), b(b), c(c), d(d), dimension(dimension) { }

    virtual int getDimension() const { return dimension; }

    virtual R getValue(const Point<DS ...>& p) const override {
        double x = p.get(dimension);
        return R(a * x + b) / (c * x + d);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f(i, this);
    }

    virtual R getMin(const Interval<DS ...>& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getLowerBoundary<R>();
        else
            return std::min(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMax(const Interval<DS ...>& i) const override {
        double x = -d / c;
        if (i.getLower().get(dimension) < x && x < i.getUpper().get(dimension))
            return getUpperBoundary<R>();
        else
            return std::max(getValue(i.getLower()), getValue(i.getUpper()));
    }

    virtual R getMean(const Interval<DS ...>& i) const override {
        return R(getIntegral(i.getUpper()) - getIntegral(i.getLower())) / (i.getUpper().get(dimension) - i.getLower().get(dimension));
    }
};

template<typename R, typename ... DS>
class INET_API AdditionFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f1;
    const Ptr<const IFunction<R, DS ...>> f2;

  public:
    AdditionFunction(const Ptr<const IFunction<R, DS ...>>& f1, const Ptr<const IFunction<R, DS ...>>& f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) + f2->getValue(p);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f1->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *if1) {
            f2->partition(i1, [&] (const Interval<DS ...>& i2, const IFunction<R, DS ...> *if2) {
                // TODO: use template specialization for compile time optimization
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        ConstantFunction<R, DS ...> g(cif1->getConstantValue() + cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) + cif1->getConstantValue(), lif2->getValue(i2.getUpper()) + cif1->getConstantValue(), lif2->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) + cif2->getConstantValue(), lif1->getValue(i2.getUpper()) + cif2->getConstantValue(), lif1->getDimension());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        ASSERT(lif1->getDimension() == lif2->getDimension());
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) + lif2->getValue(i2.getLower()), lif1->getValue(i2.getUpper()) + lif2->getValue(i2.getUpper()), lif1->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API SubtractionFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f1;
    const Ptr<const IFunction<R, DS ...>> f2;

  public:
    SubtractionFunction(const Ptr<const IFunction<R, DS ...>>& f1, const Ptr<const IFunction<R, DS ...>>& f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) - f2->getValue(p);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f1->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *if1) {
            f2->partition(i1, [&] (const Interval<DS ...>& i2, const IFunction<R, DS ...> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        ConstantFunction<R, DS ...> g(cif1->getConstantValue() - cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) - cif1->getConstantValue(), lif2->getValue(i2.getUpper()) - cif1->getConstantValue(), lif2->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) - cif2->getConstantValue(), lif1->getValue(i2.getUpper()) - cif2->getConstantValue(), lif1->getDimension());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        ASSERT(lif1->getDimension() == lif2->getDimension());
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) - lif2->getValue(i2.getLower()), lif1->getValue(i2.getUpper()) - lif2->getValue(i2.getUpper()), lif1->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API MultiplicationFunction : public FunctionBase<R, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f1;
    const Ptr<const IFunction<double, DS ...>> f2;

  public:
    MultiplicationFunction(const Ptr<const IFunction<R, DS ...>>& f1, const Ptr<const IFunction<double, DS ...>>& f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) * f2->getValue(p);
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        f1->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *if1) {
            f2->partition(i1, [&] (const Interval<DS ...>& i2, const IFunction<double, DS ...> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, DS ...> *>(if2)) {
                        ConstantFunction<R, DS ...> g(cif1->getConstantValue() * cif2->getConstantValue());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<double, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif2->getValue(i2.getLower()) * cif1->getConstantValue(), lif2->getValue(i2.getUpper()) * cif1->getConstantValue(), lif2->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<double, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<R, DS ...> g(i2.getLower(), i2.getUpper(), lif1->getValue(i2.getLower()) * cif2->getConstantValue(), lif1->getValue(i2.getUpper()) * cif2->getConstantValue(), lif1->getDimension());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<double, DS ...> *>(if2)) {
                        // QuadraticFunction<double, DS ...> g(); // TODO:
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
};

template<typename R, typename ... DS>
class INET_API DivisionFunction : public FunctionBase<double, DS ...>
{
  protected:
    const Ptr<const IFunction<R, DS ...>> f1;
    const Ptr<const IFunction<R, DS ...>> f2;

  public:
    DivisionFunction(const Ptr<const IFunction<R, DS ...>>& f1, const Ptr<const IFunction<R, DS ...>>& f2) : f1(f1), f2(f2) { }

    virtual double getValue(const Point<DS ...>& p) const override {
        return unit(f1->getValue(p) / f2->getValue(p)).get();
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<double, DS ...> *)> f) const override {
        f1->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *if1) {
            f2->partition(i1, [&] (const Interval<DS ...>& i2, const IFunction<R, DS ...> *if2) {
                if (auto cif1 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        ConstantFunction<double, DS ...> g(unit(cif1->getConstantValue() / cif2->getConstantValue()).get());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        ReciprocalFunction<double, DS ...> g(0, toDouble(cif1->getConstantValue()), lif2->getA(), lif2->getB(), lif2->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lif1 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if1)) {
                    if (auto cif2 = dynamic_cast<const ConstantFunction<R, DS ...> *>(if2)) {
                        LinearInterpolatedFunction<double, DS ...> g(i2.getLower(), i2.getUpper(), unit(lif1->getValue(i2.getLower()) / cif2->getConstantValue()).get(), unit(lif1->getValue(i2.getUpper()) / cif2->getConstantValue()).get(), lif1->getDimension());
                        f(i2, &g);
                    }
                    else if (auto lif2 = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(if2)) {
                        ASSERT(lif1->getDimension() == lif2->getDimension());
                        ReciprocalFunction<double, DS ...> g(lif1->getA(), lif1->getB(), lif2->getA(), lif2->getB(), lif2->getDimension());
                        f(i2, &g);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API SumFunction : public FunctionBase<R, DS ...>
{
  protected:
    std::vector<Ptr<const IFunction<R, DS ...>>> fs;

  public:
    SumFunction() { }
    SumFunction(const std::vector<Ptr<const IFunction<R, DS ...>>>& fs) : fs(fs) { }

    const std::vector<Ptr<const IFunction<R, DS ...>>>& getElements() const { return fs; }

    virtual void addElement(const Ptr<const IFunction<R, DS ...>>& f) {
        fs.push_back(f);
    }

    virtual void removeElement(const Ptr<const IFunction<R, DS ...>>& f) {
        fs.erase(std::remove(fs.begin(), fs.end(), f), fs.end());
    }

    virtual R getValue(const Point<DS ...>& p) const override {
        R sum = R(0);
        for (auto f : fs)
            sum += f->getValue(p);
        return sum;
    }

    virtual void partition(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f) const override {
        ConstantFunction<R, DS ...> g(R(0));
        partition(0, i, f, &g);
    }

    virtual void partition(int index, const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>&, const IFunction<R, DS ...> *)> f, const IFunction<R, DS ...> *g) const {
        if (index == (int)fs.size()) {
            f(i, g);
        }
        else
            fs[index]->partition(i, [&] (const Interval<DS ...>& i1, const IFunction<R, DS ...> *h) {
                if (auto cg = dynamic_cast<const ConstantFunction<R, DS ...> *>(g)) {
                    if (auto ch = dynamic_cast<const ConstantFunction<R, DS ...> *>(h)) {
                        ConstantFunction<R, DS ...> j(cg->getConstantValue() + ch->getConstantValue());
                        partition(index + 1, i1, f, &j);
                    }
                    else if (auto lh = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(h)) {
                        LinearInterpolatedFunction<R, DS ...> j(i1.getLower(), i1.getUpper(), lh->getValue(i1.getLower()) + cg->getConstantValue(), lh->getValue(i1.getUpper()) + cg->getConstantValue(), lh->getDimension());
                        partition(index + 1, i1, f, &j);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else if (auto lg = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(g)) {
                    if (auto ch = dynamic_cast<const ConstantFunction<R, DS ...> *>(h)) {
                        LinearInterpolatedFunction<R, DS ...> j(i1.getLower(), i1.getUpper(), lg->getValue(i1.getLower()) + ch->getConstantValue(), lg->getValue(i1.getUpper()) + ch->getConstantValue(), lg->getDimension());
                        partition(index + 1, i1, f, &j);
                    }
                    else if (auto lh = dynamic_cast<const LinearInterpolatedFunction<R, DS ...> *>(h)) {
                        ASSERT(lg->getDimension() == lh->getDimension());
                        LinearInterpolatedFunction<R, DS ...> j(i1.getLower(), i1.getUpper(), lg->getValue(i1.getLower()) + lh->getValue(i1.getLower()), lg->getValue(i1.getUpper()) + lh->getValue(i1.getUpper()), lg->getDimension());
                        partition(index + 1, i1, f, &j);
                    }
                    else
                        throw cRuntimeError("TODO");
                }
                else
                    throw cRuntimeError("TODO");
            });
    }
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_FUNCTIONS_H_

