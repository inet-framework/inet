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

#ifndef __INET_MATH_DOMAIN_H_
#define __INET_MATH_DOMAIN_H_

#include <algorithm>
#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

/**
 * This class represents the domain of a mathematical function.
 */
template<typename ... T>
class INET_API Domain
{
  public:
    typedef Point<T ...> P;
    typedef Interval<T ...> I;
};

namespace internal {

template<typename ... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Domain<T ...>& d, integer_sequence<size_t, IS...>) {
    (void)std::initializer_list<bool>{(os << (IS == 0 ? "" : ", "), printUnit(os, T()), true) ... };
    return os;
}

} // namespace internal

template<typename ... T>
inline std::ostream& operator<<(std::ostream& os, const Domain<T ...>& d) {
    os << "(";
    internal::print(os, d, index_sequence_for<T ...>{});
    os << ")";
    return os;
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_IFUNCTION_H_

