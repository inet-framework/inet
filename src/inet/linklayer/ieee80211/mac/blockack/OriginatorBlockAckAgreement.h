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

#ifndef INET_ORIGINATORBLOCKACKAGREEMENT_H
#define INET_ORIGINATORBLOCKACKAGREEMENT_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreementHandler;

class OriginatorBlockAckAgreement : public cObject
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

        virtual ~OriginatorBlockAckAgreement() { }

        virtual int getBufferSize() const { return bufferSize; }
        virtual SequenceNumberCyclic getStartingSequenceNumber() { return startingSequenceNumber; }
        virtual void setStartingSequenceNumber(SequenceNumberCyclic sequenceNumber) { startingSequenceNumber = sequenceNumber; }
        virtual bool getIsAddbaResponseReceived() const { return isAddbaResponseReceived; }
        virtual bool getIsAddbaRequestSent() const { return isAddbaRequestSent; }
        virtual bool getIsAMsduSupported() const { return isAMsduSupported; }
        virtual bool getIsDelayedBlockAckPolicySupported() const { return isDelayedBlockAckPolicySupported; }
        virtual MacAddress getReceiverAddr() const { return receiverAddr; }
        virtual Tid getTid() const { return tid; }
        virtual const simtime_t getBlockAckTimeoutValue() const  { return blockAckTimeoutValue; }
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

#endif // ifndef __INET_ORIGINATORBLOCKACKAGREEMENT_H
