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
    // The caller owns these complete units. Their order follows the previous window across sequence wrap.
    typedef std::vector<std::pair<SequenceNumber, Fragments>> ReleasedFrames;

  protected:
    std::map<std::pair<Tid, MacAddress>, ReceiveBuffer *> receiveBuffers;

  protected:
    ReleasedFrames collectCompletePrecedingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber);
    ReleasedFrames collectConsecutiveCompleteFollowingMpdus(ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber);

    // Compatibility lookup for external subclasses. The returned fragments remain owned by the buffer.
    // Production delivery uses ReleasedFrames. This helper remains available for one release of deprecation.
    [[deprecated("Use the ordered ReleasedFrames result from the public DATA or BAR method.")]]
    std::vector<Packet *> getEarliestCompleteMsduOrAMsduIfExists(ReceiveBuffer *receiveBuffer);
    bool isComplete(const Fragments& fragments);
    void passedUp(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, SequenceNumberCyclic sequenceNumber);
    void releaseReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, const ReleasedFrames& releasedFrames);
    ReceiveBuffer *createReceiveBufferIfNecessary(RecipientBlockAckAgreement *agreement);
    bool addMsduIfComplete(ReceiveBuffer *receiveBuffer, ReleasedFrames& releasedFrames, SequenceNumberCyclic seqNum);

  private:
    ReleasedFrames advanceReceiveBuffer(RecipientBlockAckAgreement *agreement, ReceiveBuffer *receiveBuffer, SequenceNumberCyclic startingSequenceNumber);

  public:
    virtual ~BlockAckReordering();
    ReleasedFrames detachAgreement(MacAddress peer, Tid tid, bool deliverComplete, std::vector<Packet *>& discarded);
    void clearPeer(MacAddress peer);

    void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba);
    ReleasedFrames processReceivedQoSFrame(RecipientBlockAckAgreement *agreement, Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, bool interframe = false);
    ReleasedFrames processReceivedBlockAckReq(RecipientBlockAckAgreement *agreement, const Ptr<const Ieee80211BlockAckReq>& blockAckReq);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

