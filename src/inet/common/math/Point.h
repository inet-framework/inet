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

#ifndef __INET_MATH_POINT_H_
#define __INET_MATH_POINT_H_

#include "inet/common/IndexSequence.h"

namespace inet {

namespace math {

template<typename T>
inline double toDouble(T v) { return v.get(); }
template<>
inline double toDouble(double v) { return v; }
template<>
inline double toDouble(simtime_t v) { return v.dbl(); }

template<typename ... T>
class INET_API Point : public std::tuple<T ...>
{
  protected:
    template<size_t ... IS>
    double getImpl(int index, integer_sequence<size_t, IS...>) const {
        double result = 0;
        std::initializer_list<double>({ result = (IS == index ? toDouble(std::get<IS>(*this)) : result) ... });
        return result;
    }

    template<size_t ... IS>
    Point<T ...> add(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) + std::get<IS>(o)) ... };
    }

    template<size_t ... IS>
    Point<T ...> subtract(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) - std::get<IS>(o)) ... };
    }

    template<size_t ... IS>
    Point<T ...> multiply(double d, integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) * d) ... };
    }

    template<size_t ... IS>
    Point<T ...> divide(double d, integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) / d) ... };
    }

    template<size_t ... IS>
    bool smaller(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        bool result = true;
        std::initializer_list<bool>({ result &= std::get<IS>(*this) < std::get<IS>(o) ... });
        return result;
    }

    template<size_t ... IS>
    bool greater(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        bool result = true;
        std::initializer_list<bool>({ result &= std::get<IS>(*this) > std::get<IS>(o) ... });
        return result;
    }

  public:
    Point(T ... t) : std::tuple<T ...>(t ...) { }

    double get(int index) const {
        return getImpl(index, index_sequence_for<T ...>{});
    }

    Point<T ...> operator+(const Point<T ...>& o) const {
        return add(o, index_sequence_for<T ...>{});
    }

    Point<T ...> operator-(const Point<T ...>& o) const {
        return subtract(o, index_sequence_for<T ...>{});
    }

    Point<T ...> operator*(double d) const {
        return multiply(d, index_sequence_for<T ...>{});
    }

    Point<T ...> operator/(double d) const {
        return divide(d, index_sequence_for<T ...>{});
    }

    bool operator<(const Point<T ...>& o) const {
        return smaller(o, index_sequence_for<T ...>{});
    }

    bool operator<=(const Point<T ...>& o) const {
        return *this == o || *this < o;
    }

    bool operator>(const Point<T ...>& o) const {
        return greater(o, index_sequence_for<T ...>{});
    }

    bool operator>=(const Point<T ...>& o) const {
        return *this == o || *this > o;
    }
};

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

namespace internal {

template<std::size_t ... NS, typename T, typename ... TS>
inline Point<TS ...> tailImpl(index_sequence<NS ...>, const Point<T, TS ...>& p) {
   return Point<TS ...>( std::get<NS + 1u>(p) ... );
}

template<typename ... TS1, size_t ... IS1, typename ... TS2, size_t ... IS2>
inline Point<TS1 ..., TS2 ...> concatImpl(const Point<TS1 ...>& p1, integer_sequence<size_t, IS1 ...>, const Point<TS2 ...>& p2, integer_sequence<size_t, IS2 ...>) {
    return Point<TS1 ..., TS2 ...> { (std::get<IS1>(p1)) ..., (std::get<IS2>(p2)) ... };
}

template<typename ... T>
inline Point<T ...> getLowerBoundaries() {
    return Point<T ...>{ (getLowerBoundary<T>()) ... };
}

template<typename ... T>
inline Point<T ...> getUpperBoundaries() {
    return Point<T ...>{ (getUpperBoundary<T>()) ... };
}

template<typename ... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Point<T ...>& p, integer_sequence<size_t, IS...>) {
    std::initializer_list<bool> { (os << (IS == 0 ? "" : ", ") << std::get<IS>(p) ) ... };
    return os;
}

}

template<typename T, typename ... TS>
T head(const Point<T, TS ...>& p) {
   return std::get<0>(p);
}

template<typename T, typename ... TS>
Point<TS ...> tail(const Point<T, TS ...>& p) {
   return internal::tailImpl(make_index_sequence<sizeof...(TS)>() , p);
}

template<typename ... TS1, typename ... TS2>
Point<TS1 ..., TS2 ...> concat(const Point<TS1 ...>& p1, const Point<TS2 ...>& p2) {
    return internal::concatImpl(p1, index_sequence_for<TS1 ...>{}, p2, index_sequence_for<TS2 ...>{});
}

template<typename ... T>
inline Point<T ...> getLowerBoundaries() {
    return internal::getLowerBoundaries<T ...>();
}

template<typename ... T>
inline Point<T ...> getUpperBoundaries() {
    return internal::getUpperBoundaries<T ...>();
}

template<typename T0>
inline std::ostream& operator<<(std::ostream& os, const Point<T0>& p) {
    return os << std::get<0>(p);
}

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Point<T ...>& p) {
    os << "("; internal::print(os, p, index_sequence_for<T ...>{}); os << ")";
    return os;
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_POINT_H_

