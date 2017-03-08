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

#ifndef __INET_BLOCKACKREORDERING_H
#define __INET_BLOCKACKREORDERING_H

#include "inet/linklayer/ieee80211/mac/blockackreordering/ReceiveBuffer.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

//
// Figure 5-1—MAC data plane architecture -- Block Ack Reordering
// 9.21.4 Receive buffer operation
//
class INET_API BlockAckReordering
{
    public:
        typedef std::vector<Ieee80211DataFrame *> Fragments;
        typedef std::map<SequenceNumber, Fragments> ReorderBuffer;

    protected:
        std::map<std::pair<Tid, MACAddress>, ReceiveBuffer *> receiveBuffers;

    protected:
        ReorderBuffer collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, int startingSequenceNumber);
        ReorderBuffer collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, int startingSequenceNumber);

        std::vector<Ieee80211DataFrame*> getEarliestCompleteMsduOrAMsduIfExists(ReceiveBuffer *receiveBuffer);
        bool isComplete(const Fragments& fragments);
        void passedUp(ReceiveBuffer *receiveBuffer, int sequenceNumber);
        void releaseReceiveBuffer(ReceiveBuffer *receiveBuffer, const ReorderBuffer& reorderBuffer);
        ReceiveBuffer* createReceiveBufferIfNecessary(RecipientBlockAckAgreement *agreement);
        bool addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReorderBuffer &reorderBuffer, SequenceNumber seqNum);

    public:
        virtual ~BlockAckReordering();

        void processReceivedDelba(Ieee80211Delba *delba);
        ReorderBuffer processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Ieee80211DataFrame* dataFrame);
        ReorderBuffer processReceivedBlockAckReq(Ieee80211BlockAckReq* frame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_BLOCKACKREORDERING_H
