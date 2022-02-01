//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

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

