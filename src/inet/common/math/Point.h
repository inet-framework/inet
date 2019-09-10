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

#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/IndexSequence.h"
#include "inet/common/Units.h"

namespace inet {

namespace math {

template<typename T>
inline void outputUnit(std::ostream& os, T v) { units::output_unit<typename T::unit>::fn(os); }
template<>
inline void outputUnit(std::ostream& os, double v) { os << "unit"; }
template<>
inline void outputUnit(std::ostream& os, simtime_t v) { os << "s"; }
template<>
inline void outputUnit(std::ostream& os, Quaternion v) { os << "quaternion"; }

template<typename T>
inline double toDouble(T v) { return v.get(); }
template<>
inline double toDouble(simsec v) { return v.get().dbl(); }
template<>
inline double toDouble(double v) { return v; }
template<>
inline double toDouble(simtime_t v) { return v.dbl(); }

template<typename T>
inline T getLowerBoundary() { return T(-INFINITY); }
template<>
inline simsec getLowerBoundary() { return simsec(-SimTime::getMaxTime() / 2); }
template<>
inline double getLowerBoundary() { return -INFINITY; }
template<>
inline simtime_t getLowerBoundary() { return -SimTime::getMaxTime() / 2; }

template<typename T>
inline T getUpperBoundary() { return T(INFINITY); }
template<>
inline simsec getUpperBoundary() { return simsec(SimTime::getMaxTime() / 2); }
template<>
inline double getUpperBoundary() { return INFINITY; }
template<>
inline simtime_t getUpperBoundary() { return SimTime::getMaxTime() / 2; }

namespace internal {

template<int DIMS, int SIZE>
struct bits_to_indices_sequence { };

template<>
struct bits_to_indices_sequence<0b0, 0> { typedef integer_sequence<size_t> type; };

template<>
struct bits_to_indices_sequence<0b0, 1> { typedef integer_sequence<size_t> type; };

template<>
struct bits_to_indices_sequence<0b00, 2> { typedef integer_sequence<size_t, 0> type; };
template<>
struct bits_to_indices_sequence<0b01, 2> { typedef integer_sequence<size_t, 1> type; };
template<>
struct bits_to_indices_sequence<0b10, 2> { typedef integer_sequence<size_t, 0> type; };

template<int DIMS, int SIZE>
using make_bits_to_indices_sequence = typename bits_to_indices_sequence<DIMS, SIZE>::type;

template<typename S, size_t ... SIS, typename D, size_t ... DIS>
void copyTupleElements(const S& source, integer_sequence<size_t, SIS ...>, D& destination, integer_sequence<size_t, DIS ...>) {
    (void)std::initializer_list<double>{ toDouble(std::get<DIS>(destination) = std::get<SIS>(source)) ... };
}

} // namespace internal

template<typename ... T>
class INET_API Point : public std::tuple<T ...>
{
  public:
    typedef std::tuple<T ...> type;

  protected:
    template<size_t ... IS>
    double getImpl(int index, integer_sequence<size_t, IS...>) const {
        double result = 0;
        (void)std::initializer_list<double>{ result = (IS == index ? toDouble(std::get<IS>(*this)) : result) ... };
        return result;
    }

    template<size_t ... IS>
    void setImpl(int index, double value, integer_sequence<size_t, IS...>) {
        (void)std::initializer_list<double>{ (IS == index ? toDouble(std::get<IS>(*this) = T(value)) : 0) ... };
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
        (void)std::initializer_list<bool>{ result &= std::get<IS>(*this) < std::get<IS>(o) ... };
        return result;
    }

    template<size_t ... IS>
    bool smallerOrEqual(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        bool result = true;
        (void)std::initializer_list<bool>{ result &= std::get<IS>(*this) <= std::get<IS>(o) ... };
        return result;
    }

    template<size_t ... IS>
    bool greater(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        bool result = true;
        (void)std::initializer_list<bool>{ result &= std::get<IS>(*this) > std::get<IS>(o) ... };
        return result;
    }

    template<size_t ... IS>
    bool greaterOrEqual(const Point<T ...>& o, integer_sequence<size_t, IS ...>) const {
        bool result = true;
        (void)std::initializer_list<bool>{ result &= std::get<IS>(*this) >= std::get<IS>(o) ... };
        return result;
    }

  public:
    Point(T ... t) : std::tuple<T ...>(t ...) { }

    double get(int index) const {
        return getImpl(index, index_sequence_for<T ...>{});
    }

    void set(int index, double value) {
        return setImpl(index, value, index_sequence_for<T ...>{});
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
        return smallerOrEqual(o, index_sequence_for<T ...>{});
    }

    bool operator>(const Point<T ...>& o) const {
        return greater(o, index_sequence_for<T ...>{});
    }

    bool operator>=(const Point<T ...>& o) const {
        return greaterOrEqual(o, index_sequence_for<T ...>{});
    }

    template<typename P, int DIMS>
    void copyTo(P& p) const {
        internal::copyTupleElements(*this, index_sequence_for<T ...>{}, p, internal::make_bits_to_indices_sequence<DIMS, std::tuple_size<typename P::type>::value>{});
    }

    template<typename P, int DIMS>
    void copyFrom(const P& p) {
        internal::copyTupleElements(p, internal::make_bits_to_indices_sequence<DIMS, std::tuple_size<typename P::type>::value>{}, *this, index_sequence_for<T ...>{});
    }

    static Point<T ...> getZero() {
        return Point<T ...>{ T(0) ... };
    }

    static Point<T ...> getLowerBoundaries() {
        return Point<T ...>{ (getLowerBoundary<T>()) ... };
    }

    static Point<T ...> getUpperBoundaries() {
        return Point<T ...>{ (getUpperBoundary<T>()) ... };
    }
};

namespace internal {

template<std::size_t ... NS, typename T, typename ... TS>
inline Point<TS ...> tailImpl(index_sequence<NS ...>, const Point<T, TS ...>& p) {
   return Point<TS ...>( std::get<NS + 1u>(p) ... );
}

template<typename ... TS1, size_t ... IS1, typename ... TS2, size_t ... IS2>
inline Point<TS1 ..., TS2 ...> concatImpl(const Point<TS1 ...>& p1, integer_sequence<size_t, IS1 ...>, const Point<TS2 ...>& p2, integer_sequence<size_t, IS2 ...>) {
    return Point<TS1 ..., TS2 ...> { (std::get<IS1>(p1)) ..., (std::get<IS2>(p2)) ... };
}

template<typename ... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Point<T ...>& p, integer_sequence<size_t, IS...>) {
    (void)std::initializer_list<bool>{(os << (IS == 0 ? "" : ", ") << std::get<IS>(p), true) ... };
    return os;
}

} // namespace internal

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

