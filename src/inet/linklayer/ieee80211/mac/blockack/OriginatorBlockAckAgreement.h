//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORBLOCKACKAGREEMENT_H
#define __INET_ORIGINATORBLOCKACKAGREEMENT_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreementHandler;

class INET_API OriginatorBlockAckAgreement : public cObject
{
  protected:
    MacAddress receiverAddr = MacAddress::UNSPECIFIED_ADDRESS;
    Tid tid = -1;
    int numSentBaPolicyFrames = 0;
    SequenceNumberCyclic startingSequenceNumber;
    int bufferSize = -1;
    bool isAMsduSupported = false;
    bool isDelayedBlockAckPolicySupported = false;
    bool isAddbaResponseReceived = false;
    bool isAddbaRequestSent = false;
    simtime_t blockAckTimeoutValue = -1;
    simtime_t expirationTime = -1;

  public:
    OriginatorBlockAckAgreement(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize, bool isAMsduSupported, bool isDelayedBlockAckPolicySupported) :
        receiverAddr(receiverAddr),
        tid(tid),
        startingSequenceNumber(startingSequenceNumber),
        bufferSize(bufferSize),
        isAMsduSupported(isAMsduSupported),
        isDelayedBlockAckPolicySupported(isDelayedBlockAckPolicySupported)
    {
    }

    virtual ~OriginatorBlockAckAgreement() {}

    virtual int getBufferSize() const { return bufferSize; }
    virtual SequenceNumberCyclic getStartingSequenceNumber() { return startingSequenceNumber; }
    virtual void setStartingSequenceNumber(SequenceNumberCyclic sequenceNumber) { startingSequenceNumber = sequenceNumber; }
    virtual bool getIsAddbaResponseReceived() const { return isAddbaResponseReceived; }
    virtual bool getIsAddbaRequestSent() const { return isAddbaRequestSent; }
    virtual bool getIsAMsduSupported() const { return isAMsduSupported; }
    virtual bool getIsDelayedBlockAckPolicySupported() const { return isDelayedBlockAckPolicySupported; }
    virtual MacAddress getReceiverAddr() const { return receiverAddr; }
    virtual Tid getTid() const { return tid; }
    virtual const simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
    virtual int getNumSentBaPolicyFrames() const { return numSentBaPolicyFrames; }

    virtual void setBufferSize(int bufferSize) { this->bufferSize = bufferSize; }
    virtual void setIsAddbaResponseReceived(bool isAddbaResponseReceived) { this->isAddbaResponseReceived = isAddbaResponseReceived; }
    virtual void setIsAddbaRequestSent(bool isAddbaRequestSent) { this->isAddbaRequestSent = isAddbaRequestSent; }
    virtual void setIsAMsduSupported(bool isAMsduSupported) { this->isAMsduSupported = isAMsduSupported; }
    virtual void setIsDelayedBlockAckPolicySupported(bool isDelayedBlockAckPolicySupported) { this->isDelayedBlockAckPolicySupported = isDelayedBlockAckPolicySupported; }
    virtual void setBlockAckTimeoutValue(const simtime_t blockAckTimeoutValue) { this->blockAckTimeoutValue = blockAckTimeoutValue; }

    virtual void baPolicyFrameSent() { numSentBaPolicyFrames++; }
    virtual void calculateExpirationTime() { expirationTime = blockAckTimeoutValue == 0 ? SIMTIME_MAX : simTime() + blockAckTimeoutValue; }
    virtual simtime_t getExpirationTime() { return expirationTime; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

