//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DYMODEFS_H
#define __INET_DYMODEFS_H

namespace inet {

namespace dymo {

// TODO use generic MANET port
#define DYMO_UDP_PORT    269

typedef uint32_t DymoSequenceNumber;

enum DymoRouteState {
    ACTIVE,
    IDLE,
    EXPIRED,
    BROKEN,
    TIMED
};

} // namespace dymo

} // namespace inet

#endif

