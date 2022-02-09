//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTRACT_CLOCKTIME_H
#define __INET_CONTRACT_CLOCKTIME_H

#include "inet/common/INETDefs.h"

#ifdef INET_WITH_CLOCK
#include "inet/clock/common/ClockTime.h"
#define CLOCKTIME_AS_SIMTIME(x)    (x).asSimTime()
#define SIMTIME_AS_CLOCKTIME(x)    ClockTime::from(x)
#else
#define ClockTime                  SimTime
#define CLOCKTIME_AS_SIMTIME(x)    (x)
#define SIMTIME_AS_CLOCKTIME(x)    (x)
#endif // INET_WITH_CLOCK

namespace inet {

typedef ClockTime clocktime_t;

/**
 * The maximum representable simulation time with the current resolution.
 */
#define CLOCKTIME_MAX     ClockTime::getMaxTime()

/**
 * Constant for zero simulation time. Using CLOCKTIME_ZERO can be more efficient
 * than using the 0 constant.
 */
#define CLOCKTIME_ZERO    ClockTime::ZERO

} // namespace inet

#endif

