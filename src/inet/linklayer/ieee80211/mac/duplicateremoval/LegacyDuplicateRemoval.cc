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

#include "LegacyDuplicateRemoval.h"
#include "inet/common/stlutils.h"

namespace inet {
namespace ieee80211 {

bool LegacyDuplicateRemoval::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    const MACAddress& address = frame->getTransmitterAddress();
    SequenceControlField seqVal(frame);
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache.insert(std::pair<MACAddress, SequenceControlField>(address, seqVal));
    else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && frame->getRetry())
        return true;
    else
        it->second = seqVal;
    return false;
}

} // namespace ieee80211
} // namespace inet
