//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_IEEE80211DEFS_H
#define __INET_IEEE80211DEFS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

typedef int8_t FragmentNumber;
typedef int8_t Tid;

struct SequenceNumber
{
  private:
    int16_t value;

  private:
    int16_t modulo4096(int16_t value) const { ASSERT(value != -1); return (value % 4096 + 4096) % 4096; } // always returns positive result
    int16_t distance4096(int16_t other) const { ASSERT(0 <= other && other < 4096); return (value - other + 4096) % 4096; }

  public:
    SequenceNumber() : value(-1) { }
    SequenceNumber(int16_t value) : value(value) { ASSERT(0 <= value && value < 4096); }
    SequenceNumber(const SequenceNumber& other) : value(other.value) { ASSERT(other.value != -1); }

    int16_t getRaw() const { ASSERT(value != -1); return value; }

    SequenceNumber& operator=(const SequenceNumber& other) { ASSERT(other.value != -1 || value == other.value); value = other.value; return *this; }
    bool operator==(const SequenceNumber& other) const { ASSERT(value != -1 && other.value != -1); return value == other.value; }
    bool operator!=(const SequenceNumber& other) const { ASSERT(value != -1 && other.value != -1); return value != other.value; }

    SequenceNumber operator-(int other) const { ASSERT(value != -1); SequenceNumber result; result.value = modulo4096(value - other); return result; }
    SequenceNumber operator+(int other) const { ASSERT(value != -1); SequenceNumber result; result.value = modulo4096(value + other); return result; }

    bool operator<(const SequenceNumber& other) const { ASSERT(value != -1 && other.value != -1);
        int16_t d = (other.value - value + 4096) % 4096;
        return 0 < d && d < 2048;
    }
    bool operator<=(const SequenceNumber& other) const { return *this == other || *this < other; }
    bool operator>(const SequenceNumber& other) const { return other < *this; }
    bool operator>=(const SequenceNumber& other) const { return *this == other || *this > other; }
};

inline std::ostream& operator<<(std::ostream& os, const SequenceNumber& sequenceNumber)
{
    os << sequenceNumber.getRaw();
    return os;
}

} // namespace ieee80211
} // namespace inet

#endif

