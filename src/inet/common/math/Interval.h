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

#include <bitset>
#include <functional>
#include <numeric>
#include <type_traits>
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

template<typename ... T>
class INET_API Interval
{
  protected:
    Point<T ...> lower;
    Point<T ...> upper;
    unsigned int closed; // 1 bit per dimension, upper end (lower end is always closed in all dimensions)

  protected:
    template<size_t ... IS>
    void checkImpl(integer_sequence<size_t, IS...>) const {
        std::initializer_list<bool> bs({ std::get<IS>(upper) < std::get<IS>(lower) ... });
        if (std::any_of(bs.begin(), bs.end(), [] (bool b) { return b; }))
            throw cRuntimeError("Invalid arguments");
        if (closed != (closed & ((1 << std::tuple_size<std::tuple<T ...>>::value) - 1)))
            throw cRuntimeError("Invalid arguments");
    }

    template<size_t ... IS>
    bool containsImpl(const Point<T ...>& p, integer_sequence<size_t, IS...>) const {
        bool result = lower < p;
        if (result) {
            unsigned int b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
            std::initializer_list<bool>({ result &= ((closed & (b >> IS)) ? std::get<IS>(p) <= std::get<IS>(upper) : std::get<IS>(p) < std::get<IS>(upper)) ... });
        }
        return result;
    }

    template<size_t ... IS>
    Interval<T ...> intersectImpl(const Interval<T ...>& o, integer_sequence<size_t, IS...>) const {
        unsigned int b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        Point<T ...> l( std::max(std::get<IS>(lower), std::get<IS>(o.lower)) ... );
        Point<T ...> u( std::min(std::get<IS>(upper), std::get<IS>(o.upper)) ... );
        unsigned int c = 0;
        std::initializer_list<unsigned int>({ c += ((b >> IS) & (std::get<IS>(lower) > std::get<IS>(u) || std::get<IS>(upper) < std::get<IS>(l) ? 0 :
                                                                (std::get<IS>(upper) == std::get<IS>(o.upper) ? (closed & o.closed) :
                                                                (std::get<IS>(upper) < std::get<IS>(o.upper) ? closed : o.closed)))) ... });
        Point<T ...> l1( std::min(std::get<IS>(upper), std::get<IS>(l)) ... );
        Point<T ...> u1( std::max(std::get<IS>(lower), std::get<IS>(u)) ... );
        return Interval<T ...>(l1, u1, c);
    }

    template<size_t ... IS>
    double getVolumeImpl(integer_sequence<size_t, IS...>) const {
        double result = 1;
        unsigned int b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        std::initializer_list<double>({ result *= ((closed & (b >> IS)) && std::get<IS>(upper) == std::get<IS>(lower) ? 1 : toDouble(std::get<IS>(upper) - std::get<IS>(lower))) ... });
        return result;
    }

    template<size_t ... IS>
    bool isEmptyImpl(integer_sequence<size_t, IS...>) const {
        unsigned int b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        bool result = false;
        std::initializer_list<bool>({ result |= ((closed & (b >> IS)) ? false : std::get<IS>(lower) == std::get<IS>(upper)) ... });
        return result;
    }

  public:
    Interval(const Point<T ...>& lower, const Point<T ...>& upper, unsigned int closed) : lower(lower), upper(upper), closed(closed) {
#ifndef NDEBUG
        checkImpl(index_sequence_for<T ...>{});
#endif
    }

    const Point<T ...>& getLower() const { return lower; }
    const Point<T ...>& getUpper() const { return upper; }
    unsigned int getClosed() const { return closed; }

    bool contains(const Point<T ...>& p) const {
        return containsImpl(p, index_sequence_for<T ...>{});
    }

    Interval<T ...> intersect(const Interval<T ...>& o) const {
        return intersectImpl(o, index_sequence_for<T ...>{});
    }

    double getVolume() const {
        return getVolumeImpl(index_sequence_for<T ...>{});
    }

    bool isEmpty() const {
        return isEmptyImpl(index_sequence_for<T ...>{});
    }
};

template<typename T0>
void iterateBoundaries(const Interval<T0>& i, const std::function<void (const Point<T0>&)> f) {
    f(i.getLower());
    f(i.getUpper());
}

template<typename T0, typename ... TS>
typename std::enable_if<sizeof ... (TS) != 0, void>::type
iterateBoundaries(const Interval<T0, TS ...>& i, const std::function<void (const Point<T0, TS ...>&)> f) {
    Interval<TS ...> i1(tail(i.getLower()), tail(i.getUpper()), i.getClosed() >> 1);
    iterateBoundaries<TS ...>(i1, std::function<void (const Point<TS ...>&)>([&] (const Point<TS ...>& q) {
        f(concat(Point<T0>(head(i.getLower())), q));
        f(concat(Point<T0>(head(i.getUpper())), q));
    }));
}

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Interval<T ...>& i) {
    return os << "[" << i.getLower() << " ... " << i.getUpper() << "] " << std::bitset<std::tuple_size<std::tuple<T ...>>::value>(i.getClosed());
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERVAL_H_

