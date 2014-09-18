//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_DYMODEFS_H
#define __INET_DYMODEFS_H

namespace inet {

namespace dymo {

// TODO: use generic MANET port
#define DYMO_UDP_PORT    269

typedef uint32_t DYMOSequenceNumber;

// TODO: metric types are defined in a separate [RFC 6551]
enum DYMOMetricType {
    HOP_COUNT = 3,    // Hop Count has Metric Type assignment 3
};

enum DYMORouteState {
    ACTIVE,
    IDLE,
    EXPIRED,
    BROKEN,
    TIMED
};

} // namespace dymo

} // namespace inet

#endif // ifndef __INET_DYMODEFS_H

