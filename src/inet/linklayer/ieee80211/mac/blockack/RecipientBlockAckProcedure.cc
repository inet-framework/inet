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

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "RecipientBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

//
// Upon successful reception of a frame of a type that requires an immediate BlockAck response, the receiving
// STA shall transmit a BlockAck frame after a SIFS period, without regard to the busy/idle state of the medium.
// The rules that specify the contents of this BlockAck frame are defined in 9.21.
//
void RecipientBlockAckProcedure::processReceivedBlockAckReq(Ieee80211BlockAckReq* blockAckReq, IRecipientQoSAckPolicy *ackPolicy, IRecipientBlockAckAgreementHandler* blockAckAgreementHandler, IProcedureCallback *callback)
{
    numReceivedBlockAckReq++;
    if (auto basicBlockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(blockAckReq)) {
        auto agreement = blockAckAgreementHandler->getAgreement(basicBlockAckReq->getTidInfo(), basicBlockAckReq->getTransmitterAddress());
        if (ackPolicy->isBlockAckNeeded(basicBlockAckReq, agreement)) {
            auto blockAck = buildBlockAck(basicBlockAckReq, agreement);
            blockAck->setDuration(ackPolicy->computeBasicBlockAckDurationField(basicBlockAckReq));
            callback->transmitControlResponseFrame(blockAck, basicBlockAckReq);
        }
    }
    else
        throw cRuntimeError("Unsupported BlockAckReq");
}

void RecipientBlockAckProcedure::processTransmittedBlockAck(Ieee80211BlockAck* blockAck)
{
    numSentBlockAck++;
}

//
// The Basic BlockAck frame contains acknowledgments for the MPDUs of up to 64 previous MSDUs. In the
// Basic BlockAck frame, the STA acknowledges only the MPDUs starting from the starting sequence control
// until the MPDU with the highest sequence number that has been received, and the STA shall set bits in the
// Block Ack bitmap corresponding to all other MPDUs to 0.
//
Ieee80211BlockAck* RecipientBlockAckProcedure::buildBlockAck(Ieee80211BlockAckReq* blockAckReq, RecipientBlockAckAgreement *agreement)
{
    if (auto basicBlockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(blockAckReq)) {
        ASSERT(agreement != nullptr);
        Ieee80211BasicBlockAck *blockAck = new Ieee80211BasicBlockAck("BasicBlockAck");
        int startingSequenceNumber = basicBlockAckReq->getStartingSequenceNumber();
        for (SequenceNumber seqNum = startingSequenceNumber; seqNum < startingSequenceNumber + 64; seqNum++) {
            BitVector &bitmap = blockAck->getBlockAckBitmap(seqNum - startingSequenceNumber);
            for (FragmentNumber fragNum = 0; fragNum < 16; fragNum++) {
                bool ackState = agreement->getBlockAckRecord()->getAckState(seqNum, fragNum);
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
