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

#ifndef __INET_RECIPIENTBLOCKACKAGREEMENT_H
#define __INET_RECIPIENTBLOCKACKAGREEMENT_H

#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreement : public cObject
{
    protected:
        BlockAckRecord *blockAckRecord = nullptr;

        SequenceNumberCyclic startingSequenceNumber;
        int bufferSize = -1;
        simtime_t blockAckTimeoutValue = 0;
        bool isAddbaResponseSent = false;
        simtime_t expirationTime = -1;

    public:
        RecipientBlockAckAgreement(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber, int bufferSize, simtime_t blockAckTimeoutValue);
        virtual ~RecipientBlockAckAgreement() { delete blockAckRecord; }

        virtual void blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header);

        virtual BlockAckRecord *getBlockAckRecord() const { return blockAckRecord; }
        virtual simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
        virtual int getBufferSize() const { return bufferSize; }
        virtual SequenceNumberCyclic getStartingSequenceNumber() const { return startingSequenceNumber; }

        virtual void addbaResposneSent() { isAddbaResponseSent = true; }
        virtual void calculateExpirationTime() { expirationTime = blockAckTimeoutValue == 0 ? SIMTIME_MAX : simTime() + blockAckTimeoutValue; }
        virtual simtime_t getExpirationTime() { return expirationTime; }
        friend std::ostream& operator<<(std::ostream& os, const RecipientBlockAckAgreement& agreement);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECIPIENTBLOCKACKAGREEMENT_H
