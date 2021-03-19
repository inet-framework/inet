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

typedef int8_t FragmentNumber;
typedef int8_t Tid;
typedef int16_t SequenceNumber;

struct SequenceNumberCyclic
{
  private:
    SequenceNumber value;

  private:
    SequenceNumber modulo4096(SequenceNumber value) const { ASSERT(value != -1); return (value % 4096 + 4096) % 4096; } // always returns positive result
    SequenceNumber distance4096(SequenceNumber other) const { ASSERT(0 <= other && other < 4096); return (value - other + 4096) % 4096; }

  public:
    SequenceNumberCyclic() : value(-1) {}
    explicit SequenceNumberCyclic(SequenceNumber value) : value(value) { ASSERT(0 <= value && value < 4096); }
    SequenceNumberCyclic(const SequenceNumberCyclic& other) : value(other.value) { ASSERT(other.value != -1); }

    SequenceNumber get() const { ASSERT(value != -1); return value; }

    SequenceNumberCyclic& operator=(const SequenceNumberCyclic& other) { ASSERT(other.value != -1 || value == other.value); value = other.value; return *this; }
    bool operator==(const SequenceNumberCyclic& other) const { ASSERT(value != -1 && other.value != -1); return value == other.value; }
    bool operator!=(const SequenceNumberCyclic& other) const { ASSERT(value != -1 && other.value != -1); return value != other.value; }

    SequenceNumberCyclic operator-(int other) const { ASSERT(value != -1); SequenceNumberCyclic result; result.value = modulo4096(value - other); return result; }
    SequenceNumberCyclic operator+(int other) const { ASSERT(value != -1); SequenceNumberCyclic result; result.value = modulo4096(value + other); return result; }

    bool operator<(const SequenceNumberCyclic& other) const {
        ASSERT(value != -1 && other.value != -1);
        SequenceNumber d = (other.value - value + 4096) % 4096;
        return 0 < d && d < 2048;
    }

    bool operator<=(const SequenceNumberCyclic& other) const { return *this == other || *this < other; }
    bool operator>(const SequenceNumberCyclic& other) const { return other < *this; }
    bool operator>=(const SequenceNumberCyclic& other) const { return *this == other || *this > other; }
};

inline std::ostream& operator<<(std::ostream& os, const SequenceNumberCyclic& sequenceNumber)
{
    os << sequenceNumber.get();
    return os;
}

} // namespace ieee80211
} // namespace inet

#endif
