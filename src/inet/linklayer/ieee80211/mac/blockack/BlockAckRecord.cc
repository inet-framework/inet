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

#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

namespace inet {
namespace ieee80211 {

BlockAckRecord::BlockAckRecord(MacAddress originatorAddress, Tid tid) :
    originatorAddress(originatorAddress),
    tid(tid)
{
}

void BlockAckRecord::blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    SequenceNumberCyclic sequenceNumber = header->getSequenceNumber();
    FragmentNumber fragmentNumber = header->getFragmentNumber();
    acknowledgmentState[SequenceControlField(sequenceNumber.get(), fragmentNumber)] = true;
}

bool BlockAckRecord::getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber)
{
    // The status of MPDUs that are considered “old” and prior to the sequence number
    // range for which the receiver maintains status shall be reported as successfully
    // received (i.e., the corresponding bit in the bitmap shall be set to 1).
    auto it = acknowledgmentState.find(SequenceControlField(sequenceNumber.get(), fragmentNumber));
    if (it != acknowledgmentState.end()) {
        return true;
    }
    else if (acknowledgmentState.size() == 0) {
        return true; // TODO: old?
    }
    else {
        auto earliest = acknowledgmentState.begin();
        return SequenceNumberCyclic(earliest->first.getSequenceNumber()) > sequenceNumber; // old = true
    }
}

void BlockAckRecord::removeAckStates(SequenceNumberCyclic sequenceNumber)
{
    auto it = acknowledgmentState.begin();
    while (it != acknowledgmentState.end()) {
        if (SequenceNumberCyclic(it->first.getSequenceNumber()) < sequenceNumber)
            it = acknowledgmentState.erase(it);
        else
            it++;
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
