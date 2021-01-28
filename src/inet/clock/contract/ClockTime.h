//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_CONTRACT_CLOCKTIME_H
#define __INET_CONTRACT_CLOCKTIME_H

#include "inet/common/INETDefs.h"

namespace inet {

#ifdef INET_WITH_CLOCK
class ClockTime;
#define CLOCKTIME_AS_SIMTIME(x)    (x).asSimTime()
#define SIMTIME_AS_CLOCKTIME(x)    ClockTime::from(x)
#else
#define ClockTime                  SimTime
#define CLOCKTIME_AS_SIMTIME(x)    (x)
#define SIMTIME_AS_CLOCKTIME(x)    (x)
#endif // INET_WITH_CLOCK

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

