//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BLOCKACKREORDERING_H
#define __INET_BLOCKACKREORDERING_H

#include "inet/common/packet/Packet.h"
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
    typedef std::vector<Packet *> Fragments;
    typedef std::map<SequenceNumber, Fragments> ReorderBuffer;

  protected:
    std::map<std::pair<Tid, MacAddress>, ReceiveBuffer *> receiveBuffers;

  protected:
    ReorderBuffer collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber);
    ReorderBuffer collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber);

    std::vector<Packet *> getEarliestCompleteMsduOrAMsduIfExists(ReceiveBuffer *receiveBuffer);
    bool isComplete(const Fragments& fragments);
    void passedUp(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, SequenceNumberCyclic sequenceNumber);
    void releaseReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, const ReorderBuffer& reorderBuffer);
    ReceiveBuffer *createReceiveBufferIfNecessary(RecipientBlockAckAgreement *agreement);
    bool addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReorderBuffer& reorderBuffer, SequenceNumberCyclic seqNum);

  public:
    virtual ~BlockAckReordering();

    void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba);
    ReorderBuffer processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader);
    ReorderBuffer processReceivedBlockAckReq(RecipientBlockAckAgreement *agreement, const Ptr<const Ieee80211BlockAckReq>& blockAckReq);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

