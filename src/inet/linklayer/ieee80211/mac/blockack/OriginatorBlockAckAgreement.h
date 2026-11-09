//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORBLOCKACKAGREEMENT_H
#define __INET_ORIGINATORBLOCKACKAGREEMENT_H

#include <vector>

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
    bool responseAMsduSupported = false;
    int dialogToken = -1;
    int64_t highestTransmittedDataIndex = -1;
    bool isDelayedBlockAckPolicySupported = false;
    bool isAddbaResponseReceived = false;
    bool isAddbaRequestSent = false;
    simtime_t blockAckTimeoutValue = -1;
    simtime_t expirationTime = -1;
    bool pendingProbe = false;

  public:
    bool hasPendingProbe() const { return pendingProbe; }
    void setPendingProbe(bool value) { pendingProbe = value; }
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
    int getWindowSize() const { return std::min(bufferSize, 64); }
    static int computeSequenceDistance(SequenceNumberCyclic start, SequenceNumberCyclic sequence)
    {
        return (sequence.get() - start.get() + 4096) % 4096;
    }
    bool containsSequence(SequenceNumberCyclic sequence) const
    {
        return computeSequenceDistance(startingSequenceNumber, sequence) < getWindowSize();
    }
    // The Basic and Compressed bitmaps cover 64 sequence positions, independent of the accepted window size.
    static bool isInBitmap(SequenceNumberCyclic sequence, SequenceNumberCyclic bitmapStart)
    {
        return computeSequenceDistance(bitmapStart, sequence) < 64;
    }
    // Preserve the earliest unresolved position, or advance to the next allocation when none remains.
    void advanceWindow(SequenceNumberCyclic nextSequence, const std::vector<SequenceNumberCyclic>& unresolvedSequences)
    {
        auto start = getStartingSequenceNumber();
        int distance = computeSequenceDistance(start, nextSequence);
        for (auto sequence : unresolvedSequences)
            distance = std::min(distance, computeSequenceDistance(start, sequence));
        setStartingSequenceNumber(start + distance);
    }
    virtual SequenceNumberCyclic getStartingSequenceNumber() { return startingSequenceNumber; }
    virtual void setStartingSequenceNumber(SequenceNumberCyclic sequenceNumber) { startingSequenceNumber = sequenceNumber; }
    virtual bool getIsAddbaResponseReceived() const { return isAddbaResponseReceived; }
    virtual bool getIsAddbaRequestSent() const { return isAddbaRequestSent; }
    virtual bool getIsAMsduSupported() const { return isAMsduSupported; }
    // IEEE Std 802.11-2024, 10.25.2 defines ADDBA negotiation; INET requires both flags before A-MSDU aggregation.
    bool isAMsduPermitted() const { return isAMsduSupported && responseAMsduSupported; }
    int getDialogToken() const { return dialogToken; }
    void setDialogToken(int token) { dialogToken = token; }
    void setResponseAMsduSupported(bool value) { responseAMsduSupported = value; }
    bool getFirstDataTransmitted() const { return highestTransmittedDataIndex >= 0; }
    int64_t getHighestTransmittedDataIndex() const { return highestTransmittedDataIndex; }
    void recordTransmittedData(int64_t allocationIndex)
    {
        ASSERT(allocationIndex >= 0);
        if (highestTransmittedDataIndex < allocationIndex)
            highestTransmittedDataIndex = allocationIndex;
    }
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

