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

#ifndef __INET_MATH_INDEXSEQUENCE_H_
#define __INET_MATH_INDEXSEQUENCE_H_

#include <type_traits>
#include "inet/common/INETDefs.h"

namespace inet {

// A C++11 implementation of std::integer_sequence from C++14

template <typename T, T... N>
struct integer_sequence
{
  typedef T value_type;
  static_assert(
    std::is_integral<T>::value,
    "std::integer_sequence can only be instantiated with an integral type" );

  static inline
  std::size_t size(
  ) {
    return (sizeof...(N));
  }
};

template <std::size_t... N>
using index_sequence = integer_sequence<std::size_t, N...>;

namespace integer_sequence_detail {

template <typename T, std::size_t ..._Extra>
struct repeat;

template <typename T, T ...N, std::size_t ..._Extra>
struct repeat<integer_sequence<T, N...>, _Extra...>
{
  typedef integer_sequence<T, N...,
    1 * sizeof...(N) + N...,
    2 * sizeof...(N) + N...,
    3 * sizeof...(N) + N...,
    4 * sizeof...(N) + N...,
    5 * sizeof...(N) + N...,
    6 * sizeof...(N) + N...,
    7 * sizeof...(N) + N...,
    _Extra...> type;
};

template <std::size_t N> struct parity;
template <std::size_t N> struct make:parity<N % 8>::template pmake<N> {};

template <> struct make<0> { typedef integer_sequence<std::size_t> type; };
template <> struct make<1> { typedef integer_sequence<std::size_t, 0> type; };
template <> struct make<2> { typedef integer_sequence<std::size_t, 0, 1> type; };
template <> struct make<3> { typedef integer_sequence<std::size_t, 0, 1, 2> type; };
template <> struct make<4> { typedef integer_sequence<std::size_t, 0, 1, 2, 3> type; };
template <> struct make<5> { typedef integer_sequence<std::size_t, 0, 1, 2, 3, 4> type; };
template <> struct make<6> { typedef integer_sequence<std::size_t, 0, 1, 2, 3, 4, 5> type; };
template <> struct make<7> { typedef integer_sequence<std::size_t, 0, 1, 2, 3, 4, 5, 6> type; };

template <> struct parity<0> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type> {}; };
template <> struct parity<1> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 1> {}; };
template <> struct parity<2> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 2, N - 1> {}; };
template <> struct parity<3> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 3, N - 2, N - 1> {}; };
template <> struct parity<4> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 4, N - 3, N - 2, N - 1> {}; };
template <> struct parity<5> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 5, N - 4, N - 3, N - 2, N - 1> {}; };
template <> struct parity<6> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 6, N - 5, N - 4, N - 3, N - 2, N - 1> {}; };
template <> struct parity<7> { template <std::size_t N> struct pmake:repeat<typename make<N / 8>::type, N - 7, N - 6, N - 5, N - 4, N - 3, N - 2, N - 1> {}; };

template <typename T, typename U>
struct convert
{
  template <typename>
  struct result;

  template <T ...N>
  struct result<integer_sequence<T, N...> >
  {
    typedef integer_sequence<U, N...> type;
  };
};

template <typename T>
struct convert<T, T>
{
  template <typename U>
  struct result
  {
    typedef U type;
  };
};

template <typename T, T N>
using make_integer_sequence_unchecked =
typename convert<std::size_t, T>::template result<typename make<N>::type>::type;

template <typename T, T N>
struct make_integer_sequence
{
  static_assert(std::is_integral<T>::value,
    "std::make_integer_sequence can only be instantiated with an integral type");
  static_assert(0 <= N,"std::make_integer_sequence input shall not be negative");

  typedef make_integer_sequence_unchecked<T, N> type;
};

} // namespace integer_sequence_detail


template <typename T, T N>
using make_integer_sequence = typename integer_sequence_detail::make_integer_sequence<T, N>::type;

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // namespace inet

#endif // #ifndef __INET_MATH_INDEXSEQUENCE_H_
