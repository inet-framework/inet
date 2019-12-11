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

/**
 * N-dimensional interval (cuboid), given by its two opposite corners.
 */
template<typename ... T>
class INET_API Interval
{
  protected:
    Point<T ...> lower;
    Point<T ...> upper;
    unsigned char lowerClosed; // 1 bit per dimension
    unsigned char upperClosed; // 1 bit per dimension
    unsigned char fixed; // 1 bit per dimension, if a dimension is fixed, then lower == upper and both ends are closed

  protected:
    template<size_t ... IS>
    void checkImpl(integer_sequence<size_t, IS...>) const {
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        bool check = true;
        std::initializer_list<bool> l1{ check &= std::get<IS>(lower) <= std::get<IS>(upper) ... }; (void)l1;
        if (!check)
            throw cRuntimeError("Invalid lower or upper arguments");
        check = true;
        std::initializer_list<bool> l2{ check &= (fixed & (b >> IS) ? lowerClosed & upperClosed & (b >> IS) && std::get<IS>(lower) == std::get<IS>(upper) : true) ... }; (void)l2;
        if (!check)
            throw cRuntimeError("Invalid fixed argument, interval = %s, fixed = %d", str().c_str(), fixed);
        auto m = (1 << std::tuple_size<std::tuple<T ...>>::value) - 1;
        if (lowerClosed != (lowerClosed & m))
            throw cRuntimeError("Invalid lowerClosed argument");
        if (upperClosed != (upperClosed & m))
            throw cRuntimeError("Invalid upperClosed argument");
        if (fixed != (fixed & m))
            throw cRuntimeError("Invalid fixed argument");
        if (fixed != (lowerClosed & fixed))
            throw cRuntimeError("Invalid combination of fixed and lowerClosed arguments");
        if (fixed != (upperClosed & fixed))
            throw cRuntimeError("Invalid combination of fixed and upperClosed arguments");
    }

    template<size_t ... IS>
    bool containsImpl(const Point<T ...>& p, integer_sequence<size_t, IS...>) const {
        bool result = true;
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        (void)std::initializer_list<bool>{ result &= ((lowerClosed & (b >> IS)) ? std::get<IS>(lower) <= std::get<IS>(p) : std::get<IS>(lower) < std::get<IS>(p)) ... };
        (void)std::initializer_list<bool>{ result &= ((upperClosed & (b >> IS)) ? std::get<IS>(p) <= std::get<IS>(upper) : std::get<IS>(p) < std::get<IS>(upper)) ... };
        return result;
    }

    template<size_t ... IS>
    Interval<T ...> intersectImpl(const Interval<T ...>& o, integer_sequence<size_t, IS...>) const {
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        Point<T ...> l( std::max(std::get<IS>(lower), std::get<IS>(o.lower)) ... ); (void)l;
        Point<T ...> u( std::min(std::get<IS>(upper), std::get<IS>(o.upper)) ... ); (void)u;
        unsigned char lc = 0;
        unsigned char uc = 0;
        (void)std::initializer_list<unsigned char>{ lc += ((b >> IS) & (std::get<IS>(upper) < std::get<IS>(l) || std::get<IS>(lower) > std::get<IS>(u) ? 0 :
                                                                       (std::get<IS>(lower) == std::get<IS>(o.lower) ? (lowerClosed & o.lowerClosed) :
                                                                       (std::get<IS>(lower) > std::get<IS>(o.lower) ? lowerClosed : o.lowerClosed)))) ... };
        (void)std::initializer_list<unsigned char>{ uc += ((b >> IS) & (std::get<IS>(lower) > std::get<IS>(u) || std::get<IS>(upper) < std::get<IS>(l) ? 0 :
                                                                       (std::get<IS>(upper) == std::get<IS>(o.upper) ? (upperClosed & o.upperClosed) :
                                                                       (std::get<IS>(upper) < std::get<IS>(o.upper) ? upperClosed : o.upperClosed)))) ... };
        Point<T ...> l1( std::min(std::get<IS>(upper), std::get<IS>(l)) ... );
        Point<T ...> u1( std::max(std::get<IS>(lower), std::get<IS>(u)) ... );
        return Interval<T ...>(l1, u1, lc, uc, (fixed | o.getFixed()) & lc & uc);
    }

    template<size_t ... IS>
    double getVolumeImpl(integer_sequence<size_t, IS...>) const {
        double result = 1;
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        (void)std::initializer_list<double>{ result *= ((fixed & (b >> IS)) ? 1 : toDouble(std::get<IS>(upper) - std::get<IS>(lower))) ... };
        return result;
    }

    template<size_t ... IS>
    bool isEmptyImpl(integer_sequence<size_t, IS...>) const {
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        bool result = false;
        (void)std::initializer_list<bool>{ result |= ((fixed & (b >> IS)) ? false : std::get<IS>(lower) == std::get<IS>(upper)) ... };
        return result;
    }

  public:
    Interval(const Point<T ...>& lower, const Point<T ...>& upper, unsigned char lowerClosed, unsigned char upperClosed, unsigned char fixed) :
        lower(lower), upper(upper), lowerClosed(lowerClosed), upperClosed(upperClosed), fixed(fixed) {
#ifndef NDEBUG
        checkImpl(index_sequence_for<T ...>{});
#endif
    }

    const Point<T ...>& getLower() const { return lower; }
    const Point<T ...>& getUpper() const { return upper; }
    unsigned char getLowerClosed() const { return lowerClosed; }
    unsigned char getUpperClosed() const { return upperClosed; }
    unsigned char getFixed() const { return fixed; }

    template<typename X, int DIMENSION>
    Interval<X> get() const {
        unsigned char s = std::tuple_size<std::tuple<T ...>>::value - DIMENSION - 1;
        return Interval<X>(std::get<DIMENSION>(lower), std::get<DIMENSION>(upper), (lowerClosed >> s) & 0b1, (upperClosed >> s) & 0b1, (fixed >> s) & 0b1);
    }

    template<typename X, int DIMENSION>
    void set(const Interval<X>& ix) {
        unsigned char s = std::tuple_size<std::tuple<T ...>>::value - DIMENSION - 1;
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        auto m = (b >> DIMENSION);
        std::get<DIMENSION>(lower) = std::get<0>(ix.getLower());
        std::get<DIMENSION>(upper) = std::get<0>(ix.getUpper());
        lowerClosed = (lowerClosed & ~m) | (ix.getLowerClosed() << s);
        upperClosed = (upperClosed & ~m) | (ix.getUpperClosed() << s);
        fixed = (fixed & ~m) | (ix.getFixed() << s);
    }

    bool contains(const Point<T ...>& p) const {
        return containsImpl(p, index_sequence_for<T ...>{});
    }

    /// Returns the volume in the dimensions denoted by the 1 bits of dims
    double getVolume() const {
        return getVolumeImpl(index_sequence_for<T ...>{});
    }

    /// Returns true iff getVolume() == 0
    bool isEmpty() const {
        return isEmptyImpl(index_sequence_for<T ...>{});
    }

    Interval<T ...> getIntersected(const Interval<T ...>& o) const {
        return intersectImpl(o, index_sequence_for<T ...>{});
    }

    Interval<T ...> getShifted(const Point<T ...>& p) const {
        return Interval<T ...>(lower + p, upper + p, lowerClosed, upperClosed, fixed);
    }

    template<typename X, int DIMENSION>
    Interval<T ...> getReplaced(Interval<X> ix) const {
        Interval<T ...> i = *this;
        i.template set<X, DIMENSION>(ix);
        return i;
    }

    template<typename X, int DIMENSION>
    Interval<T ...> getFixed(X x) const {
        unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
        auto m = (b >> DIMENSION);
        Point<T ...> pl = lower;
        Point<T ...> pu = upper;
        std::get<DIMENSION>(pl) = x;
        std::get<DIMENSION>(pu) = x;
        return Interval<T ...>(pl, pu, lowerClosed | m, upperClosed | m, fixed | m);
    }

    std::string str() const {
        std::stringstream os;
        os << *this;
        return os.str();
    }
};

inline void iterateCorners(const Interval<>& i, const std::function<void (const Point<>&)> f) {
    f(Point<>());
}

template<typename T0, typename ... TS>
inline void iterateCorners(const Interval<T0, TS ...>& i, const std::function<void (const Point<T0, TS ...>&)> f) {
    Interval<TS ...> i1(tail(i.getLower()), tail(i.getUpper()), i.getLowerClosed() >> 1, i.getUpperClosed() >> 1, i.getFixed() >> 1);
    iterateCorners(i1, std::function<void (const Point<TS ...>&)>([&] (const Point<TS ...>& q) {
        f(concat(Point<T0>(head(i.getLower())), q));
        f(concat(Point<T0>(head(i.getUpper())), q));
    }));
}

namespace internal {

template<typename ... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Interval<T ...>& i, integer_sequence<size_t, IS...>) {
    const auto& lower = i.getLower();
    const auto& upper = i.getUpper();
    auto lowerClosed = i.getLowerClosed(); (void)lowerClosed;
    auto upperClosed = i.getUpperClosed(); (void)upperClosed;
    auto fixed = i.getFixed(); (void)fixed;
    unsigned char b = 1 << std::tuple_size<std::tuple<T ...>>::value >> 1;
    (void)std::initializer_list<bool>{(os << (IS == 0 ? "" : " x "), (std::get<IS>(lower) == std::get<IS>(upper) ?
            (fixed & (b >> IS) ? os << std::get<IS>(lower) : os << "[" << std::get<IS>(lower) << "]") :
            os << ((lowerClosed & (b >> IS)) ? "[" : "(") << std::get<IS>(lower) << " … " << std::get<IS>(upper) << ((upperClosed & (b >> IS)) ? "]" : ")"), true)) ... };
    return os;
}

} // namespace internal

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Interval<T ...>& i) {
    internal::print(os, i, index_sequence_for<T ...>{});
    return os;
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERVAL_H_

