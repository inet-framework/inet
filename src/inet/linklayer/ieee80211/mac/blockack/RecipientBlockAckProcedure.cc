//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckProcedure.h"

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"

namespace inet {
namespace ieee80211 {

//
// Upon successful reception of a frame of a type that requires an immediate BlockAck response, the receiving
// STA shall transmit a BlockAck frame after a SIFS period, without regard to the busy/idle state of the medium.
// The rules that specify the contents of this BlockAck frame are defined in 9.21.
//
void RecipientBlockAckProcedure::processReceivedBlockAckReq(Packet *blockAckPacketReq, const Ptr<const Ieee80211BlockAckReq>& blockAckReq, IRecipientQosAckPolicy *ackPolicy, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler, IProcedureCallback *callback)
{
    numReceivedBlockAckReq++;
    if (auto basicBlockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        auto agreement = blockAckAgreementHandler->getAgreement(basicBlockAckReq->getTidInfo(), basicBlockAckReq->getTransmitterAddress());
        if (ackPolicy->isBlockAckNeeded(basicBlockAckReq, agreement)) {
            auto blockAck = buildBlockAck(basicBlockAckReq, agreement);
            auto duration = ackPolicy->computeBasicBlockAckDurationField(blockAckPacketReq, basicBlockAckReq);
            blockAck->setDurationField(duration);
            auto blockAckPacket = new Packet("BasicBlockAck", blockAck);
            EV_DEBUG << "Duration for " << blockAckPacket->getName() << " is set to " << duration << " s.\n";
            callback->transmitControlResponseFrame(blockAckPacket, blockAck, blockAckPacketReq, basicBlockAckReq);
        }
    }
    else
        throw cRuntimeError("Unsupported BlockAckReq");
}

void RecipientBlockAckProcedure::processTransmittedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck)
{
    numSentBlockAck++;
}

//
// The Basic BlockAck frame contains acknowledgments for the MPDUs of up to 64 previous MSDUs. In the
// Basic BlockAck frame, the STA acknowledges only the MPDUs starting from the starting sequence control
// until the MPDU with the highest sequence number that has been received, and the STA shall set bits in the
// Block Ack bitmap corresponding to all other MPDUs to 0.
//
const Ptr<Ieee80211BlockAck> RecipientBlockAckProcedure::buildBlockAck(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement)
{
    if (auto basicBlockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        ASSERT(agreement != nullptr);
        auto blockAck = makeShared<Ieee80211BasicBlockAck>();
        auto startingSequenceNumber = basicBlockAckReq->getStartingSequenceNumber();
        for (int i = 0; i < 64; i++) {
            BitVector& bitmap = blockAck->getBlockAckBitmapForUpdate(i);
            for (FragmentNumber fragNum = 0; fragNum < 16; fragNum++) {
                bool ackState = agreement->getBlockAckRecord()->getAckState(startingSequenceNumber + i, fragNum);
                bitmap.setBit(fragNum, ackState);
            }
        }
        blockAck->setReceiverAddress(blockAckReq->getTransmitterAddress());
        blockAck->setCompressedBitmap(false);
        blockAck->setStartingSequenceNumber(basicBlockAckReq->getStartingSequenceNumber());
        blockAck->setTidInfo(basicBlockAckReq->getTidInfo());
        return blockAck;
    }
    else
        throw cRuntimeError("Unsupported Block Ack Request");
}

} /* namespace ieee80211 */
} /* namespace inet */

