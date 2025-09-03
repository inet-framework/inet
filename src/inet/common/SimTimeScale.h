//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SIMTIMESCALE_H
#define __INET_SIMTIMESCALE_H

#include "inet/common/NumberNear1.h"

namespace inet {

struct INET_API SimTimeScale : public NumberNear1<simtime_raw_t> {
    using Base = NumberNear1<simtime_raw_t>;
    using Base::Base;
    using Base::operator=;
    using Base::half;
    using Base::one;
    using Base::two;
    using Base::mul;
    using Base::div;

    static SimTimeScale fromPpm(double ppm) { return SimTimeScale(Base::fromPpm(ppm).raw()); }

    simtime_t mul(simtime_t t) const { return SimTime::fromRaw(Base::mul(t.raw())); }
    simtime_t div(simtime_t t) const { return SimTime::fromRaw(Base::div(t.raw())); }

    friend SimTimeScale operator*(const SimTimeScale& s1, const SimTimeScale& s2){ return SimTimeScale(s1.Base::operator*(s2).raw()); }

    friend simtime_t operator*(const simtime_t t, const SimTimeScale& s){ return s.mul(t); }
    friend simtime_t operator*(const SimTimeScale& s, const simtime_t t){ return s.mul(t); }

    friend simtime_raw_t operator*(const simtime_raw_t t, const SimTimeScale& s){ return s.Base::mul(t); }
    friend simtime_raw_t operator*(const SimTimeScale& s, const simtime_raw_t t){ return s.Base::mul(t); }

    friend simtime_t operator/(const simtime_t t, const SimTimeScale& s){ return s.div(t); }
    friend simtime_raw_t operator/(const simtime_raw_t t, const SimTimeScale& s){ return s.Base::div(t); }
};

} // namespace inet

#endif

