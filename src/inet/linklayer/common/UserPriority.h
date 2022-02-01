//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_USERPRIORITY_H
#define __INET_USERPRIORITY_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * IEEE 802.1d User Priority (UP) values
 */
enum UserPriority {
    UP_BK  = 1, // Background
    UP_BK2 = 2, // Background
    UP_BE  = 0, // Best Effort
    UP_EE  = 3, // Excellent Effort
    UP_CL  = 4, // Controlled Load
    UP_VI  = 5, // Video (<100 ms latency and jitter)
    UP_VO  = 6, // Voice (<10 ms latency and jitter)
    UP_NC  = 7  // Network Control
};

} // namespace inet

#endif

