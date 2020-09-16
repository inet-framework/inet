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

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet {
namespace ieee80211 {

SequenceControlField::SequenceControlField(SequenceNumber sequenceNumber, FragmentNumber fragmentNumber) :
    sequenceNumber(sequenceNumber),
    fragmentNumber(fragmentNumber)
{
    ASSERT(fragmentNumber < 16);
}

SequenceControlField::SequenceControlField(const Ptr<const Ieee80211DataOrMgmtHeader>& header) :
    sequenceNumber(header->getSequenceNumber()),
    fragmentNumber(header->getFragmentNumber())
{
}

bool SequenceControlField::operator <(const SequenceControlField& other) const
{
    return sequenceNumber < other.sequenceNumber ||
           (sequenceNumber == other.sequenceNumber && fragmentNumber < other.fragmentNumber);
}

} /* namespace ieee80211 */
} /* namespace inet */

