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

  protected:
    template<size_t ... IS>
    Interval<T ...> intersectImpl(const Interval<T ...>& o, integer_sequence<size_t, IS...>) const {
        Point<T ...> l({ (std::max(std::get<IS>(lower), std::get<IS>(o.lower))) } ... );
        Point<T ...> u({ (std::min(std::get<IS>(upper), std::get<IS>(o.upper))) } ... );
        return Interval<T ...>(l, u);
    }

    template<size_t ... IS>
    double getVolumeImpl(integer_sequence<size_t, IS...>) const {
        double result = 1;
        std::initializer_list<double>({ result *= toDouble(std::get<IS>(upper) - std::get<IS>(lower)) ... });
        return std::abs(result);
    }

    template<size_t ... IS>
    double getVolumeImpl(const Interval<T ...>& o, integer_sequence<size_t, IS...>) const {
        double result = 1;
        std::initializer_list<double>({ result *= (std::get<IS>(o.getLower()) != std::get<IS>(o.getUpper()) ? toDouble(std::get<IS>(upper) - std::get<IS>(lower)) : 1) ... });
        return std::abs(result);
    }

  public:
    Interval(const Point<T ...>& lower, const Point<T ...>& upper) : lower(lower), upper(upper) {
    }

    const Point<T ...>& getLower() const { return lower; }
    const Point<T ...>& getUpper() const { return upper; }

    bool contains(const Point<T ...>& p) const {
        return lower <= p && p <= upper;
    }

    Interval<T ...> intersect(const Interval<T ...>& o) const {
        return intersectImpl(o, index_sequence_for<T ...>{});
    }

    double getVolume() const {
        return getVolumeImpl(index_sequence_for<T ...>{});
    }

    double getVolume(const Interval<T ...>& o) const {
        return getVolumeImpl(o, index_sequence_for<T ...>{});
    }
};

template<typename T0>
void iterateBoundaries(const Interval<T0>& i, const std::function<void (const Point<T0>&)> f) {
    f(i.getLower());
    f(i.getUpper());
}

template<typename T0, typename ... TS>
void iterateBoundaries(const Interval<T0, TS ...>& i, const std::function<void (const Point<T0, TS ...>&)> f) {
    Interval<TS ...> i1(tail(i.getLower()), tail(i.getUpper()));
    iterateBoundaries(i1, std::function<void (const Point<TS ...>&)>([&] (const Point<TS ...>& q) {
        f(concat(Point<T0>(head(i.getLower())), q));
        f(concat(Point<T0>(head(i.getUpper())), q));
    }));
}

namespace internal {

template<typename ... T, size_t ... IS>
inline bool isValidInterval(const Interval<T ...>& i, integer_sequence<size_t, IS...>) {
    std::initializer_list<bool> bs{ (std::get<IS>(i.getLower()) < std::get<IS>(i.getUpper())) ... };
    return std::all_of(bs.begin(), bs.end(), [] (bool b) { return b; });
}

}

template<typename ... T>
bool isValidInterval(const Interval<T ...>& i) {
    return internal::isValidInterval(i, index_sequence_for<T ...>{});
}

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Interval<T ...>& i) {
    return os << "[" << i.getLower() << " ... " << i.getUpper() << "]";
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERVAL_H_

