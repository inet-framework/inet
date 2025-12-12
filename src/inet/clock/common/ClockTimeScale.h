//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_CLOCKTIMESCALE_H
#define __INET_CLOCKTIMESCALE_H

#include "inet/clock/contract/ClockTime.h"
#include "inet/common/NumberNear1.h"

namespace inet {

struct INET_API ClockTimeScale : public NumberNear1<clocktime_raw_t> {
    using Base = NumberNear1<clocktime_raw_t>;
    using Base::Base;
    using Base::operator=;
    using Base::half;
    using Base::one;
    using Base::two;
    using Base::mul;
    using Base::div;

    static ClockTimeScale fromRatio(clocktime_t numerator, clocktime_t denominator) { return ClockTimeScale(Base::fromIntegerRatio(numerator.raw(), denominator.raw()).raw()); }
    static ClockTimeScale fromDoubleRatio(double numerator, double denominator) { return ClockTimeScale(Base::fromRatio(numerator, denominator).raw()); }
    static ClockTimeScale fromDifferenceTo1(double d) { return ClockTimeScale(Base::fromDifferenceTo1(d).raw()); }
    static ClockTimeScale fromDouble(double value) { return ClockTimeScale(Base::fromDouble(value).raw()); }
    static ClockTimeScale fromPpm(double ppm) { return ClockTimeScale(Base::fromPpm(ppm).raw()); }

    clocktime_t mul(clocktime_t t) const { return ClockTime::fromRaw(Base::mul(t.raw())); }
    clocktime_t div(clocktime_t t) const { return ClockTime::fromRaw(Base::div(t.raw())); }

    friend clocktime_t operator*(const clocktime_t t, const ClockTimeScale& s){ return s.mul(t); }
    friend clocktime_t operator*(const ClockTimeScale& s, const clocktime_t t){ return s.mul(t); }

    friend clocktime_raw_t operator*(const clocktime_raw_t t, const ClockTimeScale& s){ return s.Base::mul(t); }
    friend clocktime_raw_t operator*(const ClockTimeScale& s, const clocktime_raw_t t){ return s.Base::mul(t); }

    friend clocktime_t operator/(const clocktime_t t, const ClockTimeScale& s){ return s.div(t); }

    friend clocktime_raw_t operator/(const clocktime_raw_t t, const ClockTimeScale& s){ return s.Base::div(t); }
};

} // namespace inet

#endif

