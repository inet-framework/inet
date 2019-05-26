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

#include "inet/common/INETDefs.h"

namespace inet {

namespace math {

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
{ return os << std::get<0>(p); }

template<typename T0, typename T1>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1>& p)
{ return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ")"; }

template<typename T0, typename T1, typename T2>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1, T2>& p)
{ return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << ")"; }

template<typename T0, typename T1, typename T2, typename T3>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1, T2, T3>& p)
{ return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << ", " << std::get<3>(p) << ")"; }

template<typename T0, typename T1, typename T2, typename T3, typename T4>
inline std::ostream& operator<<(std::ostream& os, const Point<T0, T1, T2, T3, T4>& p)
{ return os << "(" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << ", " << std::get<3>(p) << ", " << std::get<4>(p) << ")"; }

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_POINT_H_

