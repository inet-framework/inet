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

#ifndef __INET_IEEE80211DEFS_H
#define __INET_IEEE80211DEFS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

typedef int16_t SequenceNumber;
typedef int8_t FragmentNumber;
typedef int8_t Tid;

static SequenceNumber sequenceNumberDistance(SequenceNumber number1, SequenceNumber number2) { return (number2 - number1 + 4096) % 4096; }
static bool isSequenceNumberTooOld(SequenceNumber number, SequenceNumber nextExpected, SequenceNumber range) { return sequenceNumberDistance(nextExpected, number) > range; }
static bool isSequenceNumberLess(SequenceNumber number1, SequenceNumber number2, SequenceNumber nextExpected, SequenceNumber range) {
    int d1 = sequenceNumberDistance(nextExpected, number1);
    int d2 = sequenceNumberDistance(nextExpected, number2);
    if (d1 < range && d2 < range)
        return d1 < d2;
    else if (d1 < range && d2 >= range)
        return false;
    else if (d1 >= range && d2 < range)
        return true;
    else if (d1 >= range && d2 >= range)
        return d1 < d2;
    else {
        ASSERT(false);
        return false; // should never happen
    }
}

} // namespace ieee80211
} // namespace inet

#endif
