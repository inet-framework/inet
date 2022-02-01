//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DOMAIN_H
#define __INET_DOMAIN_H

#include <algorithm>

#include "inet/common/math/Interval.h"
#include "inet/common/math/Point.h"

namespace inet {

namespace math {

/**
 * This class represents the domain of a mathematical function.
 */
template<typename... T>
class INET_API Domain
{
  public:
    typedef Point<T ...> P;
    typedef Interval<T ...> I;
};

namespace internal {

template<typename... T, size_t ... IS>
inline std::ostream& print(std::ostream& os, const Domain<T ...>& d, std::integer_sequence<size_t, IS...>) {
    (void)std::initializer_list<bool>{ (os << (IS == 0 ? "" : ", "), printUnit(os, T()), true) ... };
    return os;
}

} // namespace internal

template<typename... T>
inline std::ostream& operator<<(std::ostream& os, const Domain<T ...>& d) {
    os << "(";
    internal::print(os, d, std::index_sequence_for<T ...>{});
    os << ")";
    return os;
}

} // namespace math

} // namespace inet

#endif

