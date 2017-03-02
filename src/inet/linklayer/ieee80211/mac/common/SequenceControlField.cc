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

#include "SequenceControlField.h"

namespace inet {
namespace ieee80211 {

SequenceControlField::SequenceControlField(SequenceNumber sequenceNumber, FragmentNumber fragmentNumber) :
    sequenceNumber(sequenceNumber),
    fragmentNumber(fragmentNumber)
{
    ASSERT(sequenceNumber < 4096);
    ASSERT(fragmentNumber < 16);
}

SequenceControlField::SequenceControlField(Ieee80211DataOrMgmtFrame* frame) :
    sequenceNumber(frame->getSequenceNumber()),
    fragmentNumber(frame->getFragmentNumber())
{
}

bool SequenceControlField::operator <(const SequenceControlField& other) const
{
    return sequenceNumber < other.sequenceNumber ||
           (sequenceNumber == other.sequenceNumber && fragmentNumber < other.fragmentNumber);
}

} /* namespace ieee80211 */
} /* namespace inet */

