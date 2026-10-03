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
    MacAddress recipientAddr;
    uint8_t dialogToken = 0;
    BlockAckRecord *blockAckRecord = nullptr;

    SequenceNumberCyclic startingSequenceNumber;
    int bufferSize = -1;
    simtime_t blockAckTimeoutValue = 0;
    bool isAMsduSupported = false;
    bool isDelayedBlockAckPolicySupported = false;
    bool isAddbaResponseSent = false;
    simtime_t expirationTime = SIMTIME_MAX;

  public:
    MacAddress getRecipientAddr() const { return recipientAddr; }
    void setRecipientAddr(MacAddress address) { recipientAddr = address; }
    uint8_t getDialogToken() const { return dialogToken; }
    void setDialogToken(uint8_t token) { dialogToken = token; }
    RecipientBlockAckAgreement(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize, simtime_t blockAckTimeoutValue);
    RecipientBlockAckAgreement(const RecipientBlockAckAgreement& other);
    virtual RecipientBlockAckAgreement *dup() const override { return new RecipientBlockAckAgreement(*this); }
    virtual ~RecipientBlockAckAgreement() { delete blockAckRecord; }

    virtual void blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header);

    virtual BlockAckRecord *getBlockAckRecord() const { return blockAckRecord; }
    virtual simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
    virtual bool getIsAMsduSupported() const { return isAMsduSupported; }
    virtual void setIsAMsduSupported(bool supported) { isAMsduSupported = supported; }
    virtual bool getIsDelayedBlockAckPolicySupported() const { return isDelayedBlockAckPolicySupported; }
    virtual int getBufferSize() const { return bufferSize; }
    virtual SequenceNumberCyclic getStartingSequenceNumber() const { return startingSequenceNumber; }

    virtual bool getIsAddbaResponseSent() const { return isAddbaResponseSent; }
    virtual void addbaResposneSent() { isAddbaResponseSent = true; calculateExpirationTime(); }
    virtual void calculateExpirationTime() { expirationTime = blockAckTimeoutValue == 0 ? SIMTIME_MAX : simTime() + blockAckTimeoutValue; }
    virtual simtime_t getExpirationTime() { return expirationTime; }
    friend std::ostream& operator<<(std::ostream& os, const RecipientBlockAckAgreement& agreement);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

