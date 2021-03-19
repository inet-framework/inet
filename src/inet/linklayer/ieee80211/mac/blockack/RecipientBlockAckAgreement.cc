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

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

RecipientBlockAckAgreement::RecipientBlockAckAgreement(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize, simtime_t lastUsedTime) :
    startingSequenceNumber(startingSequenceNumber),
    bufferSize(bufferSize),
    blockAckTimeoutValue(lastUsedTime)
{
    calculateExpirationTime();
    blockAckRecord = new BlockAckRecord(originatorAddress, tid);
}

void RecipientBlockAckAgreement::blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header)
{
    ASSERT(header->getAckPolicy() == BLOCK_ACK);
    blockAckRecord->blockAckPolicyFrameReceived(header);
}

std::ostream& operator<<(std::ostream& os, const RecipientBlockAckAgreement& agreement)
{
    os << "originator address = " << agreement.blockAckRecord->getOriginatorAddress() << ", "
    << "tid = " << agreement.blockAckRecord->getTid() << ", "
    << "starting sequence number = " << agreement.startingSequenceNumber << ", "
    << "buffer size = " << agreement.bufferSize << ", "
    << "block ack timeout value = " << agreement.blockAckTimeoutValue;
    return os;
}

} /* namespace ieee80211 */
} /* namespace inet */
