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

#ifndef __INET_SEQUENCECONTROLFIELD_H
#define __INET_SEQUENCECONTROLFIELD_H

#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * 8.2.4.4.1 Sequence Control field structure
 * The Sequence Control field is 16 bits in length and consists of two subfields, the Sequence Number and the Fragment Number.
 */
class INET_API SequenceControlField
{
    private:
        SequenceNumber sequenceNumber;
        FragmentNumber fragmentNumber;

    public:
        SequenceControlField(SequenceNumber sequenceNumber, FragmentNumber fragmentNumber);
        SequenceControlField(Ieee80211DataOrMgmtFrame* frame);

        SequenceNumber getSequenceNumber() const { return sequenceNumber; }
        FragmentNumber getFragmentNumber() const { return fragmentNumber; }
        bool operator <(const SequenceControlField& other) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_SEQUENCECONTROLFIELD_H

