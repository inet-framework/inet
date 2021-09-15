//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ACCESSCATEGORY_H
#define __INET_ACCESSCATEGORY_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * IEEE 802.11e QoS (EDCA) access categories.
 */
enum AccessCategory
{
    AC_BK = 0,
    AC_BE = 1,
    AC_VI = 2,
    AC_VO = 3,
    AC_NUMCATEGORIES
};

inline std::string printAccessCategory(AccessCategory ac)
{
    switch(ac) {
        case AC_BK : return "Background";
        case AC_BE : return "Best effort";
        case AC_VI : return "Video";
        case AC_VO : return "Voice";
        default: throw cRuntimeError("Unknown access category");
    }
}

} // namespace ieee80211
} // namespace inet

#endif

