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
#include "inet/common/math/Interpolator.h"

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
class INET_API Function : public IFunction<R, DS ...>
{
  public:
    virtual Interval<R> getRange() const { return Interval<R>(getLowerBoundary<R>(), getUpperBoundary<R>()); }
    virtual Interval<DS ...> getDomain() const { return Interval<DS ...>(getLowerBoundaries<DS ...>(), getUpperBoundaries<DS ...>()); };

    virtual Function<R, DS ...> *limitDomain(const Interval<DS ...>& i) const {
        return new DomainLimitedFunction<R, DS ...>(this, i.intersect(getDomain()));
    }

    virtual R getMin() const { return getMin(getDomain()); }
    virtual R getMin(const Interval<DS ...>& i) const {
        R result(getUpperBoundary<R>());
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            iterateBoundaries<DS ...>(i1, [&] (const Point<DS ...>& p) {
                result = std::min(this->getValue(p), result);
            });
        });
        return result;
    }

    virtual R getMax() const { return getMax(getDomain()); }
    virtual R getMax(const Interval<DS ...>& i) const {
        R result(getLowerBoundary<R>());
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            iterateBoundaries<DS ...>(i1, [&] (const Point<DS ...>& p) {
                result = std::max(this->getValue(p), result);
            });
        });
        return result;
    }

    virtual R getMean() const { return getMean(getDomain()); }
    virtual R getMean(const Interval<DS ...>& i) const {
        int count = 0;
        R result(0);
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            iterateBoundaries<DS ...>(i1, [&] (const Point<DS ...>& p) {
                count++;
                result += this->getValue(p);
            });
        });
        return result / count;
    }

    static Function<R, DS ...> *add(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) {
        return new AdditionFunction<R, DS ...>(f1, f2);
    }

    static Function<R, DS ...> *subtract(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) {
        return new SubtractionFunction<R, DS ...>(f1, f2);
    }

    static Function<R, DS ...> *multiply(const Function<R, DS ...> *f1, const Function<double, DS ...> *f2) {
        return new MultiplicationFunction<R, DS ...>(f1, f2);
    }

    static Function<double, DS ...> *divide(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) {
        return new DivisionFunction<R, DS ...>(f1, f2);
    }
};

template<typename R, typename ... DS>
inline std::ostream& operator<<(std::ostream& os, const Function<R, DS ...>& f)
{
    os << "f {" << std::endl;
    f.iterateInterpolatable(f.getDomain(), [&] (const Interval<DS ...>& i) {
        os << "  i " << i << " -> {";
        bool first = true;
        iterateBoundaries<DS ...>(i, [&] (const Point<DS ...>& p) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << "@ " << p << " = " << f.getValue(p);
        });
        os << "}" << std::endl;
    });
    return os << "}";
}

template<typename R, typename ... DS>
class INET_API DomainLimitedFunction : public Function<R, DS ...>
{
  protected:
    const Function<R, DS ...> *f;
    const Interval<DS ...> domain;

  public:
    DomainLimitedFunction(const Function<R, DS ...> *f, const Interval<DS ...>& domain) : f(f), domain(domain) { }

    virtual Interval<R> getRange() const override { return Interval<R>(this->getMin(domain), this->getMax(domain)); };
    virtual Interval<DS ...> getDomain() const override { return domain; };

    virtual R getValue(const Point<DS ...>& p) const override {
        // TODO: check that p is o domain
        return f->getValue(p);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> g) const override {
        auto i1 = i.intersect(domain);
        if (isValidInterval(i1))
            f->iterateInterpolatable(i1, g);
    }
};

template<typename R, typename ... DS>
class INET_API ConstantFunction : public Function<R, DS ...>
{
  protected:
    R r;

  public:
    ConstantFunction(R r) : r(r) { }

    virtual Interval<R> getRange() const { return Interval<R>(r, r); }

    virtual R getValue(const Point<DS ...>& p) const override { return r; }
    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override { f(i); }

    virtual R getMin(const Interval<DS ...>& i) const override { return r; }
    virtual R getMax(const Interval<DS ...>& i) const override { return r; }
    virtual R getMean(const Interval<DS ...>& i) const override { return r; }
};

template<typename R, typename ... DS>
class INET_API AdditionFunction : public Function<R, DS ...>
{
  protected:
    const Function<R, DS ...> *f1;
    const Function<R, DS ...> *f2;

  public:
    AdditionFunction(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) + f2->getValue(p);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override {
        f1->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            f2->iterateInterpolatable(i1, [&] (const Interval<DS ...>& i2) {
                f(i2);
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API SubtractionFunction : public Function<R, DS ...>
{
  protected:
    const Function<R, DS ...> *f1;
    const Function<R, DS ...> *f2;

  public:
    SubtractionFunction(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) - f2->getValue(p);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override {
        f1->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            f2->iterateInterpolatable(i1, [&] (const Interval<DS ...>& i2) {
                f(i2);
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API MultiplicationFunction : public Function<R, DS ...>
{
  protected:
    const Function<R, DS ...> *f1;
    const Function<double, DS ...> *f2;

  public:
    MultiplicationFunction(const Function<R, DS ...> *f1, const Function<double, DS ...> *f2) : f1(f1), f2(f2) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f1->getValue(p) * f2->getValue(p);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override {
        f1->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            f2->iterateInterpolatable(i1, [&] (const Interval<DS ...>& i2) {
                f(i2);
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API DivisionFunction : public Function<double, DS ...>
{
  protected:
    const Function<R, DS ...> *f1;
    const Function<R, DS ...> *f2;

  public:
    DivisionFunction(const Function<R, DS ...> *f1, const Function<R, DS ...> *f2) : f1(f1), f2(f2) { }

    virtual double getValue(const Point<DS ...>& p) const override {
        return unit(f1->getValue(p) / f2->getValue(p)).get();
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override {
        f1->iterateInterpolatable(i, [&] (const Interval<DS ...>& i1) {
            f2->iterateInterpolatable(i1, [&] (const Interval<DS ...>& i2) {
                f(i2);
            });
        });
    }
};

template<typename R, typename ... DS>
class INET_API SumFunction : public Function<R, DS ...>
{
  protected:
    std::vector<const Function<R, DS ...> *> fs;

  public:
    SumFunction(const std::vector<const Function<R, DS ...> *>& fs) : fs(fs) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        R sum = R(0);
        for (auto f : fs)
            sum += f->getValue(p);
        return sum;
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const override {
        iterateInterpolatable(0, i, f);
    }

    virtual void iterateInterpolatable(int index, const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const {
        if (index == (int)fs.size())
            f(i);
        else
            fs[index]->iterateInterpolatable(i, [&] (const Interval<DS ...>& i) {
                iterateInterpolatable(index + 1, i, f);
            });
    }
};

template<typename R, typename ... DS>
class INET_API ShiftFunction : public Function<R, DS ...>
{
  protected:
    const Function<R, DS ...> *f;
    const Point<DS ...> s;

  public:
    ShiftFunction(const Function<R, DS ...> *f, Point<DS ...> s) : f(f), s(s) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        return f->getValue(p - s);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> g) const override {
        f->iterateInterpolatable(Interval<DS ...>(i.getLower() - s, i.getUpper() - s), [&] (const Interval<DS ...>& j) {
            g(Interval<DS ...>(j.getLower() + s, j.getUpper() + s));
        });
    }
};

template<typename X, typename Y>
class INET_API OrthogonalCombinatorFunction : public Function<double, X, Y>
{
  protected:
    const Function<double, X> *f;
    const Function<double, Y> *g;

  public:
    OrthogonalCombinatorFunction(const Function<double, X> *f, const Function<double, Y> *g) : f(f), g(g) { }

    virtual double getValue(const Point<X, Y>& p) const override {
        return f->getValue(std::get<0>(p)) * g->getValue(std::get<1>(p));
    }

    virtual void iterateInterpolatable(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>& i)> h) const override {
        Interval<X> ix(Point<X>(std::get<0>(i.getLower())), Point<X>(std::get<0>(i.getUpper())));
        Interval<Y> iy(Point<Y>(std::get<1>(i.getLower())), Point<Y>(std::get<1>(i.getUpper())));
        f->iterateInterpolatable(ix, [&] (const Interval<X>& ixf) {
            g->iterateInterpolatable(iy, [&] (const Interval<Y>& iyg) {
                Point<X, Y> lower(std::get<0>(ixf.getLower()), std::get<0>(iyg.getLower()));
                Point<X, Y> upper(std::get<0>(ixf.getUpper()), std::get<0>(iyg.getUpper()));
                h(Interval<X, Y>(lower, upper));
            });
        });
    }
};

template<typename R, typename X>
class INET_API OneDimensionalBoxcarFunction : public Function<R, X>
{
  protected:
    X lower;
    X upper;
    R r;

  public:
    OneDimensionalBoxcarFunction(X lower, X upper, R r) : lower(lower), upper(upper), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const { return Interval<R>(R(0), r); }

    virtual R getValue(const Point<X>& p) const override {
        return std::get<0>(p) < lower || std::get<0>(p) > upper ? R(0) : r;
    }

    virtual void iterateInterpolatable(const Interval<X>& i, const std::function<void (const Interval<X>& i)> f) const override {
        auto i1 = i.intersect(Interval<X>(getLowerBoundary<X>(), Point<X>(upper)));
        if (isValidInterval(i1)) f(i1);
        auto i2 = i.intersect(Interval<X>(Point<X>(lower), Point<X>(upper)));
        if (isValidInterval(i2)) f(i2);
        auto i3 = i.intersect(Interval<X>(Point<X>(lower), getUpperBoundary<X>()));
        if (isValidInterval(i3)) f(i3);
    }
};

template<typename R, typename X, typename Y>
class INET_API TwoDimensionalBoxcarFunction : public Function<R, X, Y>
{
  protected:
    X lowerX;
    X upperX;
    Y lowerY;
    Y upperY;
    R r;

  protected:
    void callf(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>& i)> f) const {
        if (isValidInterval(i))
            f(i);
    }

  public:
    TwoDimensionalBoxcarFunction(X lowerX, X upperX, Y lowerY, Y upperY, R r) : lowerX(lowerX), upperX(upperX), lowerY(lowerY), upperY(upperY), r(r) {
        ASSERT(r > R(0));
    }

    virtual Interval<R> getRange() const { return Interval<R>(R(0), r); }

    virtual R getValue(const Point<X, Y>& p) const override {
        return std::get<0>(p) < lowerX || std::get<0>(p) > upperX || std::get<1>(p) < lowerY || std::get<1>(p) > upperY ? R(0) : r;
    }

    virtual void iterateInterpolatable(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>& i)> f) const override {
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), getLowerBoundary<Y>()), Point<X, Y>(X(lowerX), Y(lowerY)))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), getLowerBoundary<Y>()), Point<X, Y>(X(upperX), Y(lowerY)))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), getLowerBoundary<Y>()), Point<X, Y>(getUpperBoundary<X>(), Y(lowerY)))), f);

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), Y(lowerY)), Point<X, Y>(X(lowerX), Y(upperY)))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(lowerY)), Point<X, Y>(X(upperX), Y(upperY)))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(lowerY)), Point<X, Y>(getUpperBoundary<X>(), Y(upperY)))), f);

        callf(i.intersect(Interval<X, Y>(Point<X, Y>(getLowerBoundary<X>(), Y(upperY)), Point<X, Y>(X(lowerX), getUpperBoundary<Y>()))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(lowerX), Y(upperY)), Point<X, Y>(X(upperX), getUpperBoundary<Y>()))), f);
        callf(i.intersect(Interval<X, Y>(Point<X, Y>(X(upperX), Y(upperY)), Point<X, Y>(getUpperBoundary<X>(), getUpperBoundary<Y>()))), f);
    }
};

template<typename R, typename X>
class INET_API OneDimensionalInterpolatedFunction : public Function<R, X>
{
  protected:
    const std::map<X, std::pair<R, const Interpolator<X, R> *>> rs;

  public:
    OneDimensionalInterpolatedFunction(const std::map<X, std::pair<R, const Interpolator<X, R> *>>& rs) : rs(rs) { }

    virtual R getValue(const Point<X>& p) const override {
        X x = std::get<0>(p);
        auto it = rs.equal_range(x);
        auto& lt = it.first;
        auto& ut = it.second;
        if (lt != rs.end() && lt->first == x)
            return lt->second.first;
        else if (ut != rs.end() && ut->first == x)
            return ut->second.first;
        else {
            ASSERT(lt != rs.end() && ut != rs.end());
            lt--;
            const auto& interpolator = lt->second.second;
            return interpolator->get(lt->first, lt->second.first, ut->first, ut->second.first, x);
        }
    }

    virtual void iterateInterpolatable(const Interval<X>& i, const std::function<void (const Interval<X>& i)> f) const override {
        // TODO: make use of binary search and optimize
        X x;
        bool first = true;
        for (auto it : rs) {
            if (!first) {
                auto i1 = i.intersect(Interval<X>(Point<X>(x), Point<X>(it.first)));
                if (isValidInterval(i1))
                    f(i1);
            }
            else
                first = false;
            x = it.first;
        }
    }
};

template<typename R, typename X, typename Y>
class INET_API TwoDimensionalInterpolatedFunction : public Function<R, X, Y>
{
  protected:
    const Interpolator<T, R>& xi;
    const Interpolator<T, R>& yi;
    const std::vector<std::tuple<X, Y, R>> rs;

  public:
    TwoDimensionalInterpolatedFunction(const Interpolator<T, R>& xi, const Interpolator<T, R>& yi, const std::vector<std::tuple<X, Y, R>>& rs) :
        xi(xi), yi(yi), rs(rs) { }

    virtual R getValue(const Point<T>& p) const override {
        throw cRuntimeError("TODO");
    }

    virtual void iterateInterpolatable(const Interval<X, Y>& i, const std::function<void (const Interval<X, Y>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

template<typename R, typename D0, typename ... DS>
class INET_API FunctionInterpolatingFunction : public Function<R, DS ...>
{
  protected:
    const Interpolator<R, D0>& i;
    const std::map<D0, const Function<R, DS ...> *> fs;

  public:
    FunctionInterpolatingFunction(const Interpolator<R, D0>& i, const std::map<D0, const Function<R, DS ...> *>& fs) : i(i), fs(fs) { }

    virtual R getValue(const Point<D0, DS ...>& p) const override {
        D0 x = std::get<0>(p);
        auto lt = fs.lower_bound(x);
        auto ut = fs.upper_bound(x);
        ASSERT(lt != fs.end() && ut != fs.end());
        Point<DS ...> q;
        return i.get(lt->first, lt->second->getValue(q), ut->first, ut->second->getValue(q), x);
    }

    virtual void iterateInterpolatable(const Interval<D0, DS ...>& i, const std::function<void (const Interval<D0, DS ...>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

template<typename R, typename ... DS>
class INET_API GaussFunction : public Function<R, DS ...>
{
  protected:
    R mean;
    R stddev;

  public:
    GaussFunction(R mean, R stddev) : mean(mean), stddev(stddev) { }

    virtual R getValue(const Point<DS ...>& p) const override {
        throw cRuntimeError("TODO");
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_FUNCTIONS_H_

