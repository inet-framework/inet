//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTBLOCKACKAGREEMENT_H
#define __INET_RECIPIENTBLOCKACKAGREEMENT_H

#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreement : public cObject
{
  protected:
    BlockAckRecord *blockAckRecord = nullptr;
    uint64_t agreementId;

    SequenceNumberCyclic startingSequenceNumber;
    int bufferSize = -1;
    simtime_t blockAckTimeoutValue = 0;
    bool isAddbaResponseSent = false;
    simtime_t expirationTime = -1;

  public:
    RecipientBlockAckAgreement(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize, simtime_t blockAckTimeoutValue, uint64_t agreementId);
    virtual ~RecipientBlockAckAgreement() { delete blockAckRecord; }

    virtual void blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header);

    [[nodiscard]] uint64_t getAgreementId() const { return agreementId; }
    virtual BlockAckRecord *getBlockAckRecord() const { return blockAckRecord; }
    virtual simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
    virtual int getBufferSize() const { return bufferSize; }
    virtual SequenceNumberCyclic getStartingSequenceNumber() const { return startingSequenceNumber; }

    // IEEE Std 802.11-2024, 11.5.4: inactivity applies to the negotiated setup after its response.
    virtual void addbaResposneSent() { if (!isAddbaResponseSent) { isAddbaResponseSent = true; calculateExpirationTime(); } }
    virtual void calculateExpirationTime() { expirationTime = blockAckTimeoutValue == 0 ? SIMTIME_MAX : simTime() + blockAckTimeoutValue; }
    virtual simtime_t getExpirationTime() { return isAddbaResponseSent ? expirationTime : SIMTIME_MAX; }
    friend std::ostream& operator<<(std::ostream& os, const RecipientBlockAckAgreement& agreement);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

