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

#ifndef __INET_MATH_INTERVAL_H_
#define __INET_MATH_INTERVAL_H_

#include <functional>
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

template<typename ... T>
class INET_API Interval
{
  protected:
    Point<T ...> lower;
    Point<T ...> upper;

  public:
    Interval(const Point<T ...>& lower, const Point<T ...>& upper) : lower(lower), upper(upper) {
    }

    const Point<T ...>& getLower() const { return lower; }
    const Point<T ...>& getUpper() const { return upper; }

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
void iterateBoundaries(const Interval<T0>& i, const std::function<void (const Point<T0>& p)> f) {
    f(i.getLower());
    f(i.getUpper());
}

template<typename T0, typename T1>
void iterateBoundaries(const Interval<T0, T1>& i, const std::function<void (const Point<T0, T1>& p)> f) {
    f(Point<T0, T1>(std::get<0>(i.getLower()), std::get<1>(i.getLower())));
    f(Point<T0, T1>(std::get<0>(i.getLower()), std::get<1>(i.getUpper())));
    f(Point<T0, T1>(std::get<0>(i.getUpper()), std::get<1>(i.getLower())));
    f(Point<T0, T1>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper())));
}

template<typename T0, typename T1, typename T2>
void iterateBoundaries(const Interval<T0, T1, T2>& i, const std::function<void (const Point<T0, T1, T2>& p)> f) {
    f(Point<T0, T1, T2>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower())));
    f(Point<T0, T1, T2>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper())));
    f(Point<T0, T1, T2>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower())));
    f(Point<T0, T1, T2>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper())));
    f(Point<T0, T1, T2>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower())));
    f(Point<T0, T1, T2>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper())));
    f(Point<T0, T1, T2>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower())));
    f(Point<T0, T1, T2>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper())));
}

template<typename T0, typename T1, typename T2, typename T3>
void iterateBoundaries(const Interval<T0, T1, T2, T3>& i, const std::function<void (const Point<T0, T1, T2, T3>& p)> f) {
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower())));
    f(Point<T0, T1, T2, T3>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper())));
}

template<typename T0, typename T1, typename T2, typename T3, typename T4>
void iterateBoundaries(const Interval<T0, T1, T2, T3, T4>& i, const std::function<void (const Point<T0, T1, T2, T3, T4>& p)> f) {
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getLower()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getLower()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getLower()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getLower()), std::get<4>(i.getUpper())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getLower())));
    f(Point<T0, T1, T2, T3, T4>(std::get<0>(i.getUpper()), std::get<1>(i.getUpper()), std::get<2>(i.getUpper()), std::get<3>(i.getUpper()), std::get<4>(i.getUpper())));
}

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

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERVAL_H_

