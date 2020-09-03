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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/stlutils.h"
#include "inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.h"

namespace inet {
namespace ieee80211 {

bool LegacyDuplicateRemoval::isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    ASSERT(header->getType() != ST_DATA_WITH_QOS);
    const MacAddress& address = header->getTransmitterAddress();
    SequenceControlField seqVal(header);
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache.insert(std::pair<MacAddress, SequenceControlField>(address, seqVal));
    else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && header->getRetry())
        return true;
    else
        it->second = seqVal;
    return false;
}

} // namespace ieee80211
} // namespace inet
