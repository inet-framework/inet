//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Andras Varga, Benjamin Seregi
//

#ifndef __INET_USERPRIORITY_H
#define __INET_USERPRIORITY_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * IEEE 802.1d User Priority (UP) values
 */
enum UserPriority {
    UP_BK = 1, // Background
    UP_BK2 = 2, // Background
    UP_BE = 0, // Best Effort
    UP_EE = 3, // Excellent Effort
    UP_CL = 4, // Controlled Load
    UP_VI = 5, // Video (<100 ms latency and jitter)
    UP_VO = 6, // Voice (<10 ms latency and jitter)
    UP_NC = 7  // Network Control
};

} // namespace inet

#endif

