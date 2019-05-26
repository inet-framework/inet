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

#ifndef __INET_FUNCTION_H_
#define __INET_FUNCTION_H_

#include <algorithm>
#include <functional>
#include "inet/common/math/Interpolator.h"
#include "inet/common/Units.h"

namespace inet {

namespace math {

using namespace inet::units::values;

template<typename T>
inline T getLowerBoundary() { return T(-INFINITY); }
template<>
inline double getLowerBoundary() { return -INFINITY; }
template<>
inline simtime_t getLowerBoundary() { return -SimTime::getMaxTime() / 2; }

template<typename T>
inline T getUpperBoundary() { return T(INFINITY); }
template<>
inline double getUpperBoundary() { return INFINITY; }
template<>
inline simtime_t getUpperBoundary() { return SimTime::getMaxTime() / 2; }


template<typename ... T>
class INET_API Point : public std::tuple<T ...>
{
  protected:
    template<typename T0>
    static Point<T0> add(const Point<T0>& a, const Point<T0>& b) { return Point<T0>(std::get<0>(a) + std::get<0>(b)); }
    template<typename T0, typename T1>
    static Point<T0, T1> add(const Point<T0, T1>& a, const Point<T0, T1>& b) { return Point<T0, T1>(std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b)); }
    template<typename T0, typename T1, typename T2>
    static Point<T0, T1, T2> add(const Point<T0, T1, T2>& a, const Point<T0, T1, T2>& b) { return Point<T0, T1, T2>(std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b), std::get<2>(a) + std::get<2>(b)); }
    template<typename T0, typename T1, typename T2, typename T3>
    static Point<T0, T1, T2, T3> add(const Point<T0, T1, T2, T3>& a, const Point<T0, T1, T2, T3>& b) { return Point<T0, T1, T2, T3>(std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b), std::get<2>(a) + std::get<2>(b), std::get<3>(a) + std::get<3>(b)); }
    template<typename T0, typename T1, typename T2, typename T3, typename T4>
    static Point<T0, T1, T2, T3, T4> add(const Point<T0, T1, T2, T3, T4>& a, const Point<T0, T1, T2, T3, T4>& b) { return Point<T0, T1, T2, T3, T4>(std::get<0>(a) + std::get<0>(b), std::get<1>(a) + std::get<1>(b), std::get<2>(a) + std::get<2>(b), std::get<3>(a) + std::get<3>(b), std::get<4>(a) + std::get<4>(b)); }

    template<typename T0>
    static Point<T0> subtract(const Point<T0>& a, const Point<T0>& b) { return Point<T0>(std::get<0>(a) - std::get<0>(b)); }
    template<typename T0, typename T1>
    static Point<T0, T1> subtract(const Point<T0, T1>& a, const Point<T0, T1>& b) { return Point<T0, T1>(std::get<0>(a) - std::get<0>(b), std::get<1>(a) - std::get<1>(b)); }
    template<typename T0, typename T1, typename T2>
    static Point<T0, T1, T2> subtract(const Point<T0, T1, T2>& a, const Point<T0, T1, T2>& b) { return Point<T0, T1, T2>(std::get<0>(a) - std::get<0>(b), std::get<1>(a) - std::get<1>(b), std::get<2>(a) - std::get<2>(b)); }
    template<typename T0, typename T1, typename T2, typename T3>
    static Point<T0, T1, T2, T3> subtract(const Point<T0, T1, T2, T3>& a, const Point<T0, T1, T2, T3>& b) { return Point<T0, T1, T2, T3>(std::get<0>(a) - std::get<0>(b), std::get<1>(a) - std::get<1>(b), std::get<2>(a) - std::get<2>(b), std::get<3>(a) - std::get<3>(b)); }
    template<typename T0, typename T1, typename T2, typename T3, typename T4>
    static Point<T0, T1, T2, T3, T4> subtract(const Point<T0, T1, T2, T3, T4>& a, const Point<T0, T1, T2, T3, T4>& b) { return Point<T0, T1, T2, T3, T4>(std::get<0>(a) - std::get<0>(b), std::get<1>(a) - std::get<1>(b), std::get<2>(a) - std::get<2>(b), std::get<3>(a) - std::get<3>(b), std::get<4>(a) - std::get<4>(b)); }

  public:
    Point(T ... t) : std::tuple<T ...>(t ...) { }

    Point<T ...> operator+(const Point<T ...>& o) const {
        return add(*this, o);
    }

    Point<T ...> operator-(const Point<T ...>& o) const {
        return subtract(*this, o);
    }
};

template<typename T0>
inline Point<T0> getLowerBoundaries() { return Point<T0>(getLowerBoundary<T0>()); }
template<typename T0, typename T1>
inline Point<T0, T1> getLowerBoundaries() { return Point<T0, T1>(getLowerBoundary<T0>(), getLowerBoundary<T1>()); }
template<typename T0, typename T1, typename T2>
inline Point<T0, T1, T2> getLowerBoundaries() { return Point<T0, T1, T2>(getLowerBoundary<T0>(), getLowerBoundary<T1>(), getLowerBoundary<T2>()); }
template<typename T0, typename T1, typename T2, typename T3>
inline Point<T0, T1, T2, T3> getLowerBoundaries() { return Point<T0, T1, T2>(getLowerBoundary<T0>(), getLowerBoundary<T1>(), getLowerBoundary<T2>(), getLowerBoundary<T3>()); }
template<typename T0, typename T1, typename T2, typename T3, typename T4>
inline Point<T0, T1, T2, T3, T4> getLowerBoundaries() { return Point<T0, T1, T2, T3, T4>(getLowerBoundary<T0>(), getLowerBoundary<T1>(), getLowerBoundary<T2>(), getLowerBoundary<T3>(), getLowerBoundary<T4>()); }

template<typename T0>
inline Point<T0> getUpperBoundaries() { return Point<T0>(getUpperBoundary<T0>()); }
template<typename T0, typename T1>
inline Point<T0, T1> getUpperBoundaries() { return Point<T0, T1>(getUpperBoundary<T0>(), getUpperBoundary<T1>()); }
template<typename T0, typename T1, typename T2>
inline Point<T0, T1, T2> getUpperBoundaries() { return Point<T0, T1, T2>(getUpperBoundary<T0>(), getUpperBoundary<T1>(), getUpperBoundary<T2>()); }
template<typename T0, typename T1, typename T2, typename T3>
inline Point<T0, T1, T2, T3> getUpperBoundaries() { return Point<T0, T1, T2, T3>(getUpperBoundary<T0>(), getUpperBoundary<T1>(), getUpperBoundary<T2>(), getUpperBoundary<T3>()); }
template<typename T0, typename T1, typename T2, typename T3, typename T4>
inline Point<T0, T1, T2, T3, T4> getUpperBoundaries() { return Point<T0, T1, T2, T3, T4>(getUpperBoundary<T0>(), getUpperBoundary<T1>(), getUpperBoundary<T2>(), getUpperBoundary<T3>(), getUpperBoundary<T4>()); }

template<typename T0>
inline std::ostream& operator<<(std::ostream& os, const Point<T0>& p)
{
    return os << std::get<0>(p);
}

template<typename T0, typename T1>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1>& p)
{
    return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ")";
}

template<typename T0, typename T1, typename T2>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1, T2>& p)
{
    return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << ")";
}

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Point<T ...> *p)
{
    return os << *p;
}

template<typename ... T>
class INET_API Interval
{
  protected:
    Point<T ...> lower;
    Point<T ...> upper;

  public:
    Interval(const Point<T ...>& lower, const Point<T ...>& upper) : lower(lower), upper(upper) {
        // TODO: ASSERT(isValidInterval(*this));
    }

    const Point<T ...>& getLower() const { return lower; }
    const Point<T ...>& getUpper() const { return upper; }

    void iterateBoundaries(const std::function<void (const Point<T ...>& p)> f) const {
        f(lower);
        f(upper);
        // TODO:
    }

    template<typename T0>
    Interval<T0> intersect(const Interval<T0>& o) const {
        Point<T0> s(std::max(std::get<0>(lower), std::get<0>(o.lower)));
        Point<T0> e(std::min(std::get<0>(upper), std::get<0>(o.upper)));
        return Interval<T0>(s, e);
    }

    template<typename T0, typename T1>
    Interval<T0, T1> intersect(const Interval<T0, T1>& o) const {
        Point<T0, T1> s(std::max(std::get<0>(lower), std::get<0>(o.lower)), std::max(std::get<1>(lower), std::get<1>(o.lower)));
        Point<T0, T1> e(std::min(std::get<0>(upper), std::get<0>(o.upper)), std::min(std::get<1>(upper), std::get<1>(o.upper)));
        return Interval<T0, T1>(s, e);
    }

    template<typename T0, typename T1, typename T2>
    Interval<T0, T1, T2> intersect(const Interval<T0, T1, T2>& o) const {
        Point<T0, T1, T2> s(std::max(std::get<0>(lower), std::get<0>(o.lower)), std::max(std::get<1>(lower), std::get<1>(o.lower)), std::max(std::get<2>(lower), std::get<2>(o.lower)));
        Point<T0, T1, T2> e(std::min(std::get<0>(upper), std::get<0>(o.upper)), std::min(std::get<1>(upper), std::get<1>(o.upper)), std::min(std::get<2>(upper), std::get<2>(o.upper)));
        return Interval<T0, T1, T2>(s, e);
    }

    template<typename T0, typename T1, typename T2, typename T3>
    Interval<T0, T1, T2, T3> intersect(const Interval<T0, T1, T2, T3>& o) const {
        Point<T0, T1, T2, T3> s(std::max(std::get<0>(lower), std::get<0>(o.lower)), std::max(std::get<1>(lower), std::get<1>(o.lower)), std::max(std::get<2>(lower), std::get<2>(o.lower)), std::max(std::get<3>(lower), std::get<3>(o.lower)));
        Point<T0, T1, T2, T3> e(std::min(std::get<0>(upper), std::get<0>(o.upper)), std::min(std::get<1>(upper), std::get<1>(o.upper)), std::min(std::get<2>(upper), std::get<2>(o.upper)), std::min(std::get<3>(upper), std::get<3>(o.upper)));
        return Interval<T0, T1, T2, T3>(s, e);
    }

    template<typename T0, typename T1, typename T2, typename T3, typename T4>
    Interval<T0, T1, T2, T3, T4> intersect(const Interval<T0, T1, T2, T3, T4>& o) const {
        Point<T0, T1, T2, T3, T4> s(std::max(std::get<0>(lower), std::get<0>(o.lower)), std::max(std::get<1>(lower), std::get<1>(o.lower)), std::max(std::get<2>(lower), std::get<2>(o.lower)), std::max(std::get<3>(lower), std::get<3>(o.lower)), std::max(std::get<4>(lower), std::get<4>(o.lower)));
        Point<T0, T1, T2, T3, T4> e(std::min(std::get<0>(upper), std::get<0>(o.upper)), std::min(std::get<1>(upper), std::get<1>(o.upper)), std::min(std::get<2>(upper), std::get<2>(o.upper)), std::min(std::get<3>(upper), std::get<3>(o.upper)), std::min(std::get<4>(upper), std::get<4>(o.upper)));
        return Interval<T0, T1, T2, T3, T4>(s, e);
    }
};

template<typename T0>
bool isValidInterval(const Interval<T0>& i) { return std::get<0>(i.getLower()) < std::get<0>(i.getUpper()); }
template<typename T0, typename T1>
bool isValidInterval(const Interval<T0, T1>& i) { return std::get<0>(i.getLower()) < std::get<0>(i.getUpper()) && std::get<1>(i.getLower()) < std::get<1>(i.getUpper()); }
template<typename T0, typename T1, typename T2>
bool isValidInterval(const Interval<T0, T1, T2>& i) { return std::get<0>(i.getLower()) < std::get<0>(i.getUpper()) && std::get<1>(i.getLower()) < std::get<1>(i.getUpper()) && std::get<2>(i.getLower()) < std::get<2>(i.getUpper()); }
template<typename T0, typename T1, typename T2, typename T3>
bool isValidInterval(const Interval<T0, T1, T2, T3>& i) { return std::get<0>(i.getLower()) < std::get<0>(i.getUpper()) && std::get<1>(i.getLower()) < std::get<1>(i.getUpper()) && std::get<2>(i.getLower()) < std::get<2>(i.getUpper()) && std::get<3>(i.getLower()) < std::get<3>(i.getUpper()); }
template<typename T0, typename T1, typename T2, typename T3, typename T4>
bool isValidInterval(const Interval<T0, T1, T2, T3, T4>& i) { return std::get<0>(i.getLower()) < std::get<0>(i.getUpper()) && std::get<1>(i.getLower()) < std::get<1>(i.getUpper()) && std::get<2>(i.getLower()) < std::get<2>(i.getUpper()) && std::get<3>(i.getLower()) < std::get<3>(i.getUpper()) && std::get<4>(i.getLower()) < std::get<4>(i.getUpper()); }

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Interval<T ...>& i)
{
    return os << "[" << i.getLower() << " ... " << i.getUpper() << "]";
}

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Interval<T ...>* i)
{
    return os << *i;
}

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

/**
 * This interface represents a mathematical function from domain DS to range R.
 */
template<typename R, typename ... DS>
class INET_API IFunction
{
  public:
    virtual ~IFunction() {}

    virtual Interval<R> getRange() const = 0;
    virtual Interval<DS ...> getDomain() const = 0;

    virtual R getValue(const Point<DS ...>& p) const = 0;

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> f) const = 0;
    virtual IFunction<R, DS ...> *limitDomain(const Interval<DS ...>& i) const = 0;

    virtual R getMin() const = 0;
    virtual R getMin(const Interval<DS ...>& i) const = 0;

    virtual R getMax() const = 0;
    virtual R getMax(const Interval<DS ...>& i) const = 0;

    virtual R getAverage() const = 0;
    virtual R getAverage(const Interval<DS ...>& i) const = 0;
};

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
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i) {
            i.iterateBoundaries([&] (const Point<DS ...>& p) {
                result = std::min(this->getValue(p), result);
            });
        });
        return result;
    }

    virtual R getMax() const { return getMax(getDomain()); }
    virtual R getMax(const Interval<DS ...>& i) const {
        R result(getLowerBoundary<R>());
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i) {
            i.iterateBoundaries([&] (const Point<DS ...>& p) {
                result = std::max(this->getValue(p), result);
            });
        });
        return result;
    }

    virtual R getAverage() const { return getAverage(getDomain()); }
    virtual R getAverage(const Interval<DS ...>& i) const {
        int count = 0;
        R result(0);
        this->iterateInterpolatable(i, [&] (const Interval<DS ...>& i) {
            i.iterateBoundaries([&] (const Point<DS ...>& p) {
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
    os << "function {" << std::endl;
    f.iterateInterpolatable(f.getDomain(), [&] (const Interval<DS ...>& i) {
        os << "  interval = " << i << " -> (@lower = " << f.getValue(i.getLower()) << ", @upper = " << f.getValue(i.getUpper()) << ")" << std::endl;
    });
    return os << "}";
}

template<typename R, typename ... DS>
inline std::ostream& operator<<(std::ostream& os, const Function<R, DS ...> *f)
{
    return os << *f;
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
        return f->getValue(p);
    }

    virtual void iterateInterpolatable(const Interval<DS ...>& i, const std::function<void (const Interval<DS ...>& i)> g) const override {
        f->iterateInterpolatable(i, g);
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
    virtual R getAverage(const Interval<DS ...>& i) const override { return r; }
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
    const Interpolator<X, R>& i;
    const std::map<X, R> rs;

  public:
    OneDimensionalInterpolatedFunction(const Interpolator<X, R>& i, const std::map<X, R>& rs) : i(i), rs(rs) { }

    virtual R getValue(const Point<X>& p) const override {
        X x = std::get<0>(p);
        auto lt = rs.lower_bound(x);
        auto ut = rs.upper_bound(x);
        ASSERT(lt != rs.end() && ut != rs.end());
        return i.get(lt->first, lt->second, ut->first, ut->second, x);
    }

    virtual void iterateInterpolatable(const Interval<X>& i, const std::function<void (const Interval<X>& i)> f) const override {
        X x;
        bool first = true;
        for (auto it : rs) {
            if (!first)
                f(Interval<X>(Point<X>(x), Point<X>(it.first)));
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

class INET_API OfdmSpectralDensityFunction : public Function<double, Hz>
{
  protected:
    Hz carrierFrequency;
    Hz bandwidth;
    int numSubcarriers;

  public:
    OfdmSpectralDensityFunction(Hz carrierFrequency, Hz bandwidth, int numSubcarriers) :
        carrierFrequency(carrierFrequency), bandwidth(bandwidth), numSubcarriers(numSubcarriers) { }

    virtual double getValue(const Point<Hz>& p) const override {
        throw cRuntimeError("TODO");
    }

    virtual void iterateInterpolatable(const Interval<Hz>& i, const std::function<void (const Interval<Hz>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

class INET_API Ieee80211PowerDensityFunction : public Function<W, simtime_t, Hz>
{
  protected:
    OfdmSpectralDensityFunction *f;
    simtime_t duration;
    W power;

  public:
    Ieee80211PowerDensityFunction(OfdmSpectralDensityFunction *f, simtime_t duration, W power) :
        f(f), duration(duration), power(power) { }

    virtual W getValue(const Point<simtime_t, Hz>& p) const override {
        if (std::get<0>(p) < 0 || std::get<0>(p) > duration)
            return W(0);
        else
            return power * f->getValue(Point<Hz>(std::get<1>(p)));
    }

    virtual void iterateInterpolatable(const Interval<simtime_t, Hz>& i, const std::function<void (const Interval<simtime_t, Hz>& i)>) const override {
        throw cRuntimeError("TODO");
    }

    virtual W getMin(const Interval<simtime_t, Hz>& i) const override {
        if (std::get<0>(i.getUpper()) < 0 || std::get<0>(i.getLower()) > duration)
            return W(0);
        else
            return power * f->getMin(Interval<Hz>(std::get<1>(i.getLower()), std::get<1>(i.getUpper())));
    }

    virtual W getMax(const Interval<simtime_t, Hz>& i) const override {
        if (std::get<0>(i.getUpper()) < 0 || std::get<0>(i.getLower()) > duration)
            return W(0);
        else
            return power * f->getMax(Interval<Hz>(std::get<1>(i.getLower()), std::get<1>(i.getUpper())));
    }
};

class INET_API PropagationAndAttenuationFunction : public Function<W, m, m, m, simtime_t, Hz>
{
  protected:
    Point<m, m, m> location;
    Function<W, simtime_t, Hz> *power;
    Function<double, m, Hz> *attenuation;

  public:
    PropagationAndAttenuationFunction(Point<m, m, m> location, Function<W, simtime_t, Hz> *power, Function<double, m, Hz> *attenuation) :
        location(location), power(power), attenuation(attenuation) { }

    virtual W getValue(const Point<m, m, m, simtime_t, Hz>& p) const override {
        m dx = std::get<0>(p) - std::get<0>(location);
        m dy = std::get<1>(p) - std::get<1>(location);
        m dz = std::get<2>(p) - std::get<2>(location);
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        auto a = attenuation->getValue(Point<m, Hz>(distance, std::get<4>(p)));
        auto pw = power->getValue(Point<simtime_t, Hz>(std::get<3>(p), std::get<4>(p)));
        return a * pw;
    }

    virtual void iterateInterpolatable(const Interval<m, m, m, simtime_t, Hz>& i, const std::function<void (const Interval<m, m, m, simtime_t, Hz>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

} // namespace math

} // namespace inet

/*
class Medium
{
    Function<W, m, m, m, simtime_t, Hz> powerDensity;
};

class Transmission
{
    Function<W, simtime_t, Hz> powerDensity;
};

void Medium::transmit(Transmission *transmission)
{
    medium->powerDensity->add(new PropagationAndAttenuationFunction(transmission->getPowerDensity()));
}

void Medium::receive(Transmission *transmission)
{
    Interval *receptionInterval; // mobility + propagation + duration + listening
    Function *reception = new PropagationAndAttenuationFunction(transmission->getPowerDensity())->limitDomain(receptionInterval)
    Function *total = medium->powerDensity->limitDomain(receptionInterval);
    Function *interference = subtract(total, reception);
    Function *snir = divide(reception, interference);

    // receiver from here
    for (Interval i : symbolIntervals(receptionInterval)) {
        if (dbl() < snir2ser(snir->getAverage(i)))
            ; // mark incorrect symbol
    }
}
*/

#endif // #ifndef __INET_FUNCTION_H_

