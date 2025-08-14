//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_NUMBERNEAR1_H
#define __INET_NUMBERNEAR1_H

#include <cstdint>
#include <type_traits>
#include <limits>
#include <cassert>
#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Fixed-point scale factor x ∈ [0.5, 2] for accurate integer multiply/divide.
 * We encode e = 1 − x in signed Q1.63: e_q63 = round(e * 2^63), so e ∈ [−1, 0.5] ⇒ e_q63 ∈ [−2^63, +2^62].
 * The actual scale is x = 1 − e_q63 / 2^63, raw storage is int64_t.
 */
template<typename I>
struct INET_API NumberNear1 {
    using S64 = int64_t;
    using U64 = uint64_t;
    using S128 = __int128_t;
    using U128 = __uint128_t;

    static_assert(std::is_same<I, S64>::value || std::is_same<I, S128>::value, "I must be int64_t or __int128_t");

  protected:
    S64 e_q63 = 0;

    constexpr explicit NumberNear1(S64 e_q63) noexcept : e_q63(e_q63) {
        assert(e_q63 >= (S64 )std::numeric_limits<S64>::min());
        assert(e_q63 <= (S64(1) << 62));
    }

  public:
    constexpr NumberNear1() = default;

    NumberNear1& operator=(const NumberNear1&) = default;

    constexpr S64 raw() const noexcept { return e_q63; }

    static constexpr NumberNear1 half() noexcept { return NumberNear1(S64(1) << 62); }
    static constexpr NumberNear1 one() noexcept { return NumberNear1(S64(0)); }
    static constexpr NumberNear1 two() noexcept { return NumberNear1(std::numeric_limits<S64>::min()); }

    static NumberNear1 fromDouble(long double x) {
        if (x < 0.5L || x > 2.0L)
            throw cRuntimeError("Invalid argument: x = %Lg (must be in the range [0.5, 2.0])", x);
        long double e = 1.0L - x;
        long double scaled = e * (long double) (1ULL << 63);
        S64 e_q63 = (S64) std::llround(scaled);
        if (e_q63 > (S64(1) << 62))
            e_q63 = (S64(1) << 62);
        if (e_q63 < std::numeric_limits<S64>::min())
            e_q63 = std::numeric_limits<S64>::min();
        return NumberNear1(e_q63);
    }

    static NumberNear1 fromPpm(long double ppm) {
        if (ppm < -500000.0 || ppm > 1000000.0)
            throw cRuntimeError("Invalid argument: ppm = %Lg (must be in the range [-500000, 1000000] ppm)", ppm);

        // e_q63 = round( -(ppm) * 2^63 / 1e6 )
        const long double scaled = -ppm * ((long double)(1ULL << 63) / 1'000'000.0L);

        // Clamp to valid raw range: [-2^63, +2^62]
        const long double min_raw = (long double)std::numeric_limits<S64>::min();
        const long double max_raw = (long double)(S64(1) << 62);

        S64 e_q63;
        if (scaled <= min_raw)
            e_q63 = std::numeric_limits<S64>::min();
        else if (scaled >= max_raw)
            e_q63 = (S64(1) << 62);
        else
            e_q63 = (S64)std::llround(scaled);  // rounds to nearest

        return NumberNear1(e_q63);
    }

    NumberNear1 operator*(const NumberNear1& rhs) const {
        // r3 = r1 + r2 − round((r1*r2)/2^63), with r ≡ 1 − x in Q1.63
        S128 t = (S128)e_q63 * (S128)rhs.e_q63;                 // Q2.126
        t += (t >= 0) ? ((S128)1 << 62) : -((S128)1 << 62);     // round-to-nearest, ties away from 0
        S64 cross = (S64)(t / ((S128)1 << 63));                 // back to Q1.63 (portable)
        S128 sum = (S128)e_q63 + rhs.e_q63 - cross;             // r3 in Q1.63
        return NumberNear1((S64)sum);
    }

    I mul(I t) const { return mulRound(t); }

    I div(I t) const { return divRound(t); }

    constexpr I mulRound(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if constexpr (std::is_same<I, S64>::value) {
                if (t > 0 && t > std::numeric_limits<S64>::max() / 2)
                    return std::numeric_limits<S64>::max();
                if (t < 0 && t < std::numeric_limits<S64>::min() / 2)
                    return std::numeric_limits<S64>::min();
            }
            return (I)((S128) t * 2);
        }
        return mul_round(t, -e_q63);
    }

    constexpr I mulFloor(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if constexpr (std::is_same<I, S64>::value) {
                if (t > 0 && t > std::numeric_limits<S64>::max() / 2)
                    return std::numeric_limits<S64>::max();
                if (t < 0 && t < std::numeric_limits<S64>::min() / 2)
                    return std::numeric_limits<S64>::min();
            }
            return (I)((S128) t * 2);
        }
        return mul_floor(t, -e_q63);
    }

    constexpr I mulCeil(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if constexpr (std::is_same<I, S64>::value) {
                if (t > 0 && t > std::numeric_limits<S64>::max() / 2)
                    return std::numeric_limits<S64>::max();
                if (t < 0 && t < std::numeric_limits<S64>::min() / 2)
                    return std::numeric_limits<S64>::min();
            }
            return (I)((S128) t * 2);
        }
        return mul_ceil(t, -e_q63);
    }

    constexpr I divRound(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if (t == 0)
                return 0;
            const bool neg = (t < 0);
            const U128 a = neg ? U128(-(S128) t) : U128(t);
            const U128 mag = (a + 1) >> 1;
            const S128 y = neg ? -S128(mag) : S128(mag);
            if constexpr (std::is_same<I, S64>::value) {
                if (y > (S128) std::numeric_limits<S64>::max())
                    return std::numeric_limits<S64>::max();
                if (y < (S128) std::numeric_limits<S64>::min())
                    return std::numeric_limits<S64>::min();
            }
            return (I)y;
        }
        return div_round(t, -e_q63);
    }

    constexpr I divFloor(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if (t >= 0)
                return (I)((S128) t >> 1);
            const U128 a = U128(-(S128) t);
            const U128 mag = (a + 1) >> 1;
            return (I)(-(S128) mag);
        }
        return div_floor(t, -e_q63);
    }

    constexpr I divCeil(I t) const noexcept {
        if (e_q63 == std::numeric_limits<S64>::min()) {
            if (t >= 0)
                return (I)(((S128) t + 1) >> 1);
            const U128 a = U128(-(S128) t);
            const U128 mag = a >> 1;
            return (I)(-(S128) mag);
        }
        return div_ceil(t, -e_q63);
    }

    double toDouble() const noexcept {
        long double e = (long double) e_q63 / (long double) (1ULL << 63);
        return (double) (1.0L - e);
    }

    double diffFrom1() const noexcept {
        return (double) (-((long double) e_q63 / (long double) (1ULL << 63)));
    }

    std::string str() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

  protected:
    static constexpr U128 abs_to_u128(S128 v) noexcept {
        return v >= 0 ? U128(v) : U128(-(v + 1)) + 1;
    }

    static constexpr U64 abs_to_u64(S64 v) noexcept {
        return v >= 0 ? U64(v) : U64(- (S128)(v + 1)) + 1;
    }

    // t + round((t * e) / 2^63)
    static constexpr I mul_round(I t, S64 e) noexcept {
        if (e == 0)
            return t;

        const bool a_neg = (t < 0);
        const bool e_neg = (e < 0);
        const bool neg = a_neg ^ e_neg;
        const U128 ua = abs_to_u128((S128)t);
        const U64  b  = abs_to_u64(e);

        const U64 a_lo = (U64) ua;
        const U64 a_hi = (U64) (ua >> 64);

        const U128 p0 = U128(a_lo) * U128(b);
        const U128 p1 = U128(a_hi) * U128(b);

        const U128 A = p0 + (U128(1) << 62);
        const U128 delta_mag = (p1 << 1) + (A >> 63);

        const S128 delta = neg ? -S128(delta_mag) : S128(delta_mag);
        const S128 result = S128(t) + delta;

        if constexpr (std::is_same<I, S64>::value) {
            if (result > S128(std::numeric_limits<S64>::max()))
                return std::numeric_limits<S64>::max();
            if (result < S128(std::numeric_limits<S64>::min()))
                return std::numeric_limits<S64>::min();
        }
        return (I)result;
    }

    // floor(t * (1 + e / 2^63))
    static constexpr I mul_floor(I t, S64 e) noexcept {
        if (e == 0 || t == 0)
            return t;

        const bool a_neg = (t < 0);
        const bool e_neg = (e < 0);
        const bool prod_neg = a_neg ^ e_neg;
        const U128 ua = abs_to_u128((S128)t);
        const U64  b  = abs_to_u64(e);

        const U64 a_lo = (U64) ua;
        const U64 a_hi = (U64) (ua >> 64);

        const U128 p0 = U128(a_lo) * U128(b);
        const U128 p1 = U128(a_hi) * U128(b);

        const U128 q_floor = (p1 << 1) + (p0 >> 63);
        const bool has_rem = ((p0 & ((U128(1) << 63) - 1)) != 0);

        S128 q = S128(q_floor);
        if (prod_neg) {
            if (has_rem)
                q += 1;
            q = -q;
        }

        const S128 result = S128(t) + q;
        if constexpr (std::is_same<I, S64>::value) {
            if (result > S128(std::numeric_limits<S64>::max()))
                return std::numeric_limits<S64>::max();
            if (result < S128(std::numeric_limits<S64>::min()))
                return std::numeric_limits<S64>::min();
        }
        return (I)result;
    }

    // ceil(t * (1 + e / 2^63))
    static constexpr I mul_ceil(I t, S64 e) noexcept {
        if (e == 0 || t == 0)
            return t;

        const bool a_neg = (t < 0);
        const bool e_neg = (e < 0);
        const bool prod_neg = a_neg ^ e_neg;
        const U128 ua = abs_to_u128((S128)t);
        const U64  b  = abs_to_u64(e);

        const U64 a_lo = (U64) ua;
        const U64 a_hi = (U64) (ua >> 64);

        const U128 p0 = U128(a_lo) * U128(b);
        const U128 p1 = U128(a_hi) * U128(b);

        const U128 q_floor = (p1 << 1) + (p0 >> 63);
        const bool has_rem = ((p0 & ((U128(1) << 63) - 1)) != 0);

        S128 q = S128(q_floor);
        if (!prod_neg) {
            if (has_rem)
                q += 1;
        }
        else {
            q = -q;
        }

        const S128 result = S128(t) + q;
        if constexpr (std::is_same<I, S64>::value) {
            if (result > S128(std::numeric_limits<S64>::max()))
                return std::numeric_limits<S64>::max();
            if (result < S128(std::numeric_limits<S64>::min()))
                return std::numeric_limits<S64>::min();
        }
        return (I)result;
    }

    // nearest: y = round( t / (1 + e/2^63) )
    // implemented as floor( (|t| * 2^63 + D / 2) / D ) with 3-limb (192 / 64) long division
    static constexpr I div_round(I t, S64 e) noexcept {
        const U128 D128 = (U128(1) << 63) + U128((S128) e);
        const U64 D = (U64) D128;
        assert(D != 0);

        const bool neg = (t < 0);
        const U128 a = abs_to_u128((S128)t);

        const U64 a_lo = (U64) a;
        const U64 a_hi = (U64) (a >> 64);

        U64 n0 = a_lo << 63;
        const U64 carry0 = a_lo >> 1;

        const U64 x_lo = a_hi << 63;
        const U64 x_hi = a_hi >> 1;

        const U128 t1 = (U128) x_lo + (U128) carry0;
        U64 n1 = (U64) t1;
        const U64 carry1 = (U64) (t1 >> 64);

        const U128 t2 = (U128) x_hi + (U128) carry1;
        U64 n2 = (U64) t2;

        const U128 s0 = (U128) n0 + (U128) (D >> 1);
        n0 = (U64) s0;
        const U64 c0 = (U64) (s0 >> 64);

        const U128 s1 = (U128) n1 + (U128) c0;
        n1 = (U64) s1;
        const U64 c1 = (U64) (s1 >> 64);

        const U128 s2 = (U128) n2 + (U128) c1;
        n2 = (U64) s2;

        U128 rem = 0;
        auto step = [&](U64 limb) -> U64 {
            U128 cur = (rem << 64) | limb;
            U64 q = (U64) (cur / D);
            rem = cur % D;
            return q;
        };

        const U64 q2 = step(n2);
        const U64 q1 = step(n1);
        const U64 q0 = step(n0);

        if (q2 != 0) {
            if constexpr (std::is_same<I, S64>::value)
                return neg ? std::numeric_limits<S64>::min() : std::numeric_limits<S64>::max();
            else
                return neg ? (I)(-((S128) 1 << 127)) : (I)(((S128) 1 << 127) - 1);
        }

        const U128 mag = ((U128) q1 << 64) | (U128) q0;
        const S128 y = neg ? -(S128) mag : (S128) mag;

        if constexpr (std::is_same<I, S64>::value) {
            if (y > (S128) std::numeric_limits<S64>::max())
                return std::numeric_limits<S64>::max();
            if (y < (S128) std::numeric_limits<S64>::min())
                return std::numeric_limits<S64>::min();
        }
        return (I)y;
    }

    // floor: y = ⌊ t / (1 + e / 2^63) ⌋
    static constexpr I div_floor(I t, S64 e) noexcept {
        const U128 D128 = (U128(1) << 63) + U128((S128) e);
        const U64 D = (U64) D128;
        assert(D != 0);

        const bool neg = (t < 0);
        const U128 a = abs_to_u128((S128)t);

        const U64 a_lo = (U64) a;
        const U64 a_hi = (U64) (a >> 64);

        U64 n0 = a_lo << 63;
        const U64 carry0 = a_lo >> 1;

        const U64 x_lo = a_hi << 63;
        const U64 x_hi = a_hi >> 1;

        const U128 t1 = U128(x_lo) + U128(carry0);
        U64 n1 = (U64) t1;
        const U64 carry1 = (U64) (t1 >> 64);

        const U128 t2 = U128(x_hi) + U128(carry1);
        U64 n2 = (U64) t2;

        U128 rem = 0;
        auto step = [&](U64 limb) -> U64 {
            U128 cur = (rem << 64) | limb;
            U64 q = (U64) (cur / D);
            rem = cur % D;
            return q;
        };
        const U64 q2 = step(n2);
        const U64 q1 = step(n1);
        const U64 q0 = step(n0);

        if (q2 != 0) {
            if constexpr (std::is_same<I, S64>::value)
                return neg ? std::numeric_limits<S64>::min() : std::numeric_limits<S64>::max();
            else
                return neg ? (I)(-((S128) 1 << 127)) : (I)(((S128) 1 << 127) - 1);
        }

        const U128 mag = ((U128) q1 << 64) | (U128) q0;
        const S128 y = neg ? -(S128) mag : (S128) mag;

        if constexpr (std::is_same<I, S64>::value) {
            if (y > (S128) std::numeric_limits<S64>::max())
                return std::numeric_limits<S64>::max();
            if (y < (S128) std::numeric_limits<S64>::min())
                return std::numeric_limits<S64>::min();
        }
        return (I)y;
    }

    // ceil: y = ⌈ t / (1 + e / 2^63) ⌉
    static constexpr I div_ceil(I t, S64 e) noexcept {
        const U128 D128 = (U128(1) << 63) + U128((S128) e);
        const U64 D = (U64) D128;
        assert(D != 0);

        const bool neg = (t < 0);
        const U128 a = abs_to_u128((S128)t);

        const U64 a_lo = (U64) a;
        const U64 a_hi = (U64) (a >> 64);

        U64 n0 = a_lo << 63;
        const U64 carry0 = a_lo >> 1;

        const U64 x_lo = a_hi << 63;
        const U64 x_hi = a_hi >> 1;

        const U128 t1 = U128(x_lo) + U128(carry0);
        U64 n1 = (U64) t1;
        const U64 carry1 = (U64) (t1 >> 64);

        const U128 t2 = U128(x_hi) + U128(carry1);
        U64 n2 = (U64) t2;

        // Add ceiling bias only for T >= 0
        if (!neg) {
            const U128 s0 = U128(n0) + U128(D - 1);
            n0 = (U64) s0;
            const U64 c0 = (U64) (s0 >> 64);

            const U128 s1 = U128(n1) + U128(c0);
            n1 = (U64) s1;
            const U64 c1 = (U64) (s1 >> 64);

            const U128 s2 = U128(n2) + U128(c1);
            n2 = (U64) s2;
        }

        U128 rem = 0;
        auto step = [&](U64 limb) -> U64 {
            const U128 cur = (rem << 64) | limb;
            const U64 q = (U64) (cur / D);
            rem = cur % D;
            return q;
        };

        const U64 q2 = step(n2);
        const U64 q1 = step(n1);
        const U64 q0 = step(n0);

        if (q2 != 0) {
            if constexpr (std::is_same<I, S64>::value)
                return neg ? std::numeric_limits<S64>::min() : std::numeric_limits<S64>::max();
            else
                return neg ? (I)(-((S128) 1 << 127)) : (I)(((S128) 1 << 127) - 1);
        }

        const U128 mag = (U128(q1) << 64) | U128(q0);
        const S128 y = neg ? -S128(mag) : S128(mag);

        if constexpr (std::is_same<I, S64>::value) {
            if (y > (S128) std::numeric_limits<S64>::max())
                return std::numeric_limits<S64>::max();
            if (y < (S128) std::numeric_limits<S64>::min())
                return std::numeric_limits<S64>::min();
        }
        return (I)y;
    }
};

template<typename I>
inline I operator*(I t, const NumberNear1<I> &s) {
    return s.mul(t);
}

template<typename I>
inline I operator/(I t, const NumberNear1<I> &s) {
    return s.div(t);
}

template<typename I>
inline std::ostream& operator<<(std::ostream &os, const NumberNear1<I> &s) {
    std::ios::fmtflags f = os.flags();
    std::streamsize old_prec = os.precision();
    char old_fill = os.fill();

    const long double e_ld = static_cast<long double>(s.raw()) / static_cast<long double>(1ULL << 63);
    const long double x_ld = 1.0L - e_ld;
    const long double ppm = (-e_ld) * 1'000'000.0L;
    const long double ppb = (-e_ld) * 1'000'000'000.0L;

    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(18);

    os << "NumberNear1 {"
       << "x ≈ " << static_cast<double>(x_ld)
       << ", e ≈ " << static_cast<double>(e_ld)
       << ", ppm ≈ " << std::llround(ppm)
       << ", ppb ≈ " << std::llround(ppb)
       << ", e_q63 = " << s.raw()
       << "}";

    os.flags(f);
    os.precision(old_prec);
    os.fill(old_fill);
    return os;
}

} // namespace inet

#endif

