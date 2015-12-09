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
    AC_LEGACY,
    AC_NUMCATEGORIES
};

} // namespace ieee80211
} // namespace inet

#endif

