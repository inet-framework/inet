//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_POINT_H
#define __INET_POINT_H

#include "inet/common/Units.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

namespace math {

template<typename T>
inline void printUnit(std::ostream& os, T v) { units::output_unit<typename T::unit>::fn(os); }
template<>
inline void printUnit(std::ostream& os, double v) { os << "unit"; }
template<>
inline void printUnit(std::ostream& os, simtime_t v) { os << "s"; }
template<>
inline void printUnit(std::ostream& os, Quaternion v) { os << "quaternion"; }

template<typename T>
inline double toDouble(const T v) { return v.get(); }
template<>
inline double toDouble(const simsec v) { return v.get().dbl(); }
template<>
inline double toDouble(const double v) { return v; }
template<>
inline double toDouble(const simtime_t v) { return v.dbl(); }

template<typename T>
inline T getLowerBound() { return T(-INFINITY); }
template<>
inline simsec getLowerBound() { return simsec(-SimTime::getMaxTime() / 2); }
template<>
inline double getLowerBound() { return -INFINITY; }
template<>
inline simtime_t getLowerBound() { return -SimTime::getMaxTime() / 2; }

template<typename T>
inline T getUpperBound() { return T(INFINITY); }
template<>
inline simsec getUpperBound() { return simsec(SimTime::getMaxTime() / 2); }
template<>
inline double getUpperBound() { return INFINITY; }
template<>
inline simtime_t getUpperBound() { return SimTime::getMaxTime() / 2; }

namespace internal {

template<int DIMS, int SIZE>
struct bits_to_indices_sequence {};

template<>
struct bits_to_indices_sequence<0b0, 0>{ typedef std::integer_sequence<size_t> type; };

template<>
struct bits_to_indices_sequence<0b0, 1>{ typedef std::integer_sequence<size_t> type; };

template<>
struct bits_to_indices_sequence<0b00, 2>{ typedef std::integer_sequence<size_t, 0> type; }; // TODO shouldn't be empty set?
template<>
struct bits_to_indices_sequence<0b01, 2>{ typedef std::integer_sequence<size_t, 1> type; };
template<>
struct bits_to_indices_sequence<0b10, 2>{ typedef std::integer_sequence<size_t, 0> type; };

template<>
struct bits_to_indices_sequence<0b00011, 5>{ typedef std::integer_sequence<size_t, 3, 4> type; };
template<>
struct bits_to_indices_sequence<0b11000, 5>{ typedef std::integer_sequence<size_t, 0, 1> type; };
template<>
struct bits_to_indices_sequence<0b11110, 5>{ typedef std::integer_sequence<size_t, 0, 1, 2, 3> type; };

template<int DIMS, int SIZE>
using make_bits_to_indices_sequence = typename bits_to_indices_sequence<DIMS, SIZE>::type;

template<typename S, size_t ... SIS, typename D, size_t ... DIS>
void copyTupleElements(const S& source, std::integer_sequence<size_t, SIS ...>, D& destination, std::integer_sequence<size_t, DIS ...>) {
    (void)std::initializer_list<double>{ toDouble(std::get<DIS>(destination) = std::get<SIS>(source)) ... };
}

} // namespace internal

/**
 * N-dimensional point. Supports algebraic operations.
 */
template<typename... T>
class INET_API Point : public std::tuple<T ...>
{
  public:
    typedef std::tuple<T ...> type;

  protected:
    template<size_t ... IS>
    double getImpl(int index, std::integer_sequence<size_t, IS...>) const {
        double result = 0;
        (void)std::initializer_list<double>{ result = (IS == index ? toDouble(std::get<IS>(*this)) : result) ... };
        return result;
    }

    template<size_t ... IS>
    void setImpl(int index, double value, std::integer_sequence<size_t, IS...>) {
        (void)std::initializer_list<double>{ (IS == index ? toDouble(std::get<IS>(*this) = T(value)) : 0) ... };
    }

    template<size_t ... IS>
    Point<T ...> add(const Point<T ...>& o, std::integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) + std::get<IS>(o)) ... };
    }

    template<size_t ... IS>
    Point<T ...> subtract(const Point<T ...>& o, std::integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) - std::get<IS>(o)) ... };
    }

    template<size_t ... IS>
    Point<T ...> multiply(double d, std::integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) * d) ... };
    }

    template<size_t ... IS>
    Point<T ...> divide(double d, std::integer_sequence<size_t, IS ...>) const {
        return Point<T ...>{ (std::get<IS>(*this) / d) ... };
    }

  public:
    Point(T ... t) : std::tuple<T ...>(t...) {}

    double get(int index) const {
        return getImpl(index, std::index_sequence_for<T ...>{});
    }

    void set(int index, double value) {
        return setImpl(index, value, std::index_sequence_for<T ...>{});
    }

    Point<T ...> operator-() const {
        return multiply(-1, std::index_sequence_for<T ...>{});
    }

    Point<T ...> operator+(const Point<T ...>& o) const {
        return add(o, std::index_sequence_for<T ...>{});
    }

    Point<T ...> operator-(const Point<T ...>& o) const {
        return subtract(o, std::index_sequence_for<T ...>{});
    }

    Point<T ...> operator*(double d) const {
        return multiply(d, std::index_sequence_for<T ...>{});
    }

    Point<T ...> operator/(double d) const {
        return divide(d, std::index_sequence_for<T ...>{});
    }

    /// Copy the dimensions selected by the 1 bits of DIMS into p.
    template<typename P, int DIMS>
    void copyTo(P& p) const {
        internal::copyTupleElements(*this, std::index_sequence_for<T ...>{}, p, internal::make_bits_to_indices_sequence<DIMS, std::tuple_size<typename P::type>::value>{});
    }

    /// Copy the coordinates selected by the 1 bits of DIMS from p.
    template<typename P, int DIMS>
    void copyFrom(const P& p) {
        internal::copyTupleElements(p, internal::make_bits_to_indices_sequence<DIMS, std::tuple_size<typename P::type>::value>{}, *this, std::index_sequence_for<T ...>{});
    }

    template<typename X, int DIMENSION>
    Point<T ...> getReplaced(X x) const {
        Point<T ...> p = *this;
        std::get<DIMENSION>(p) = x;
        return p;
    }

    std::string str() const {
        std::stringstream os;
        os << *this;
        return os.str();
    }

    static Point<T ...> getZero() {
        return Point<T ...>{ T(0) ... };
    }

    static Point<T ...> getLowerBounds() {
        return Point<T ...>{ (getLowerBound<T>()) ... };
    }

    static Point<T ...> getUpperBounds() {
        return Point<T ...>{ (getUpperBound<T>()) ... };
    }
};

namespace internal {

template<std::size_t ... NS, typename T, typename... TS>
inline Point<TS ...> tailImpl(std::index_sequence<NS ...>, const Point<T, TS ...>& p) {
    return Point<TS ...>(std::get<NS + 1u>(p) ...);
}

template<typename... TS1, size_t ... IS1, typename... TS2, size_t ... IS2>
inline Point<TS1 ..., TS2 ...> concatImpl(const Point<TS1 ...>& p1, std::integer_sequence<size_t, IS1 ...>, const Point<TS2 ...>& p2, std::integer_sequence<size_t, IS2 ...>) {
    return Point<TS1 ..., TS2 ...>{ (std::get<IS1>(p1)) ..., (std::get<IS2>(p2)) ... };
}

template<typename... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Point<T ...>& p, std::integer_sequence<size_t, IS...>) {
    (void)std::initializer_list<bool>{ (os << (IS == 0 ? "" : ", ") << std::get<IS>(p), true) ... };
    return os;
}

} // namespace internal

/// Returns the first coordinate of p.
template<typename T, typename... TS>
T head(const Point<T, TS ...>& p) {
    return std::get<0>(p);
}

/// Returns all but the first coordinate of p.
template<typename T, typename... TS>
Point<TS ...> tail(const Point<T, TS ...>& p) {
    return internal::tailImpl(std::make_index_sequence<sizeof...(TS)>(), p);
}

/// Returns a point by concatenating the coordinates of p1 and p2.
template<typename... TS1, typename... TS2>
Point<TS1 ..., TS2 ...> concat(const Point<TS1 ...>& p1, const Point<TS2 ...>& p2) {
    return internal::concatImpl(p1, std::index_sequence_for<TS1 ...>{}, p2, std::index_sequence_for<TS2 ...>{});
}

template<typename T0>
inline std::ostream& operator<<(std::ostream& os, const Point<T0>& p) {
    return os << std::get<0>(p);
}

template<typename... T>
inline std::ostream& operator<<(std::ostream& os, const Point<T ...>& p) {
    os << "(";
    internal::print(os, p, std::index_sequence_for<T ...>{});
    os << ")";
    return os;
}

} // namespace math

} // namespace inet

#endif

