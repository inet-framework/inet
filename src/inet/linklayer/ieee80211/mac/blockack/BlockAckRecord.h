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

#ifndef __INET_BLOCKACKRECORD_H
#define __INET_BLOCKACKRECORD_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// The recipient shall maintain a Block Ack record consisting of originator address, TID, and a record of
// reordering buffer size indexed by the received MPDU sequence control value. This record holds the
// acknowledgment state of the data frames received from the originator.
//
class INET_API BlockAckRecord
{
    protected:
        MacAddress originatorAddress = MacAddress::UNSPECIFIED_ADDRESS;
        Tid tid = -1;
        std::map<SequenceControlField, bool> acknowledgmentState;

    public:
        BlockAckRecord(MacAddress originatorAddress, Tid tid);
        virtual ~BlockAckRecord() { }

        void blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header);
        bool getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber);
        void removeAckStates(SequenceNumberCyclic sequenceNumber);

        MacAddress getOriginatorAddress() { return originatorAddress; }
        Tid getTid() { return tid; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_BLOCKACKRECORD_H
