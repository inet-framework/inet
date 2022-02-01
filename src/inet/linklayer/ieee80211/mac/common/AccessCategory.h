//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACCESSCATEGORY_H
#define __INET_ACCESSCATEGORY_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * IEEE 802.11e QoS (EDCA) access categories.
 */
enum AccessCategory {
    AC_BK = 0,
    AC_BE = 1,
    AC_VI = 2,
    AC_VO = 3,
    AC_NUMCATEGORIES
};

inline std::string printAccessCategory(AccessCategory ac)
{
    switch (ac) {
        case AC_BK: return "Background";
        case AC_BE: return "Best effort";
        case AC_VI: return "Video";
        case AC_VO: return "Voice";
        default: throw cRuntimeError("Unknown access category");
    }
}

} // namespace ieee80211
} // namespace inet

#endif

