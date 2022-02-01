//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/TxOpFs.h"

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

/*
 * TODO add [ RTS CTS ] (txop-part-requiring-ack txop-part-providing-ack )|
 */
TxOpFs::TxOpFs() :
    // Excerpt from G.3 EDCA and HCCA sequences
    // txop-sequence =
    //   ( ( ( RTS CTS ) | CTS + self ) Data + individual + QoS +( block-ack | no-ack ) ) |
    //   [ RTS CTS ] (txop-part-requiring-ack txop-part-providing-ack )|
    //   [ RTS CTS ] (Management | ( Data + QAP )) + individual ACK |
    //   [ RTS CTS ] (BlockAckReq BlockAck ) |
    //   ht-txop-sequence;
    AlternativesFs({new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new DataFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new DataFs(),
                                      new AckFs()}), // TODO should be in txop-part-requiring-ack
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isBlockAckReqRtsCtsNeeded)),
                                      new BlockAckReqBlockAckFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new AlternativesFs({new ManagementAckFs(),
                                                          /* TODO DATA + QAP*/},
                                                         ALTERNATIVESFS_SELECTOR(selectMgmtOrDataQap))})},
                   ALTERNATIVESFS_SELECTOR(selectTxOpSequence))
{
}

int TxOpFs::selectMgmtOrDataQap(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return 0;
}

int TxOpFs::selectTxOpSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    const auto& macHeader = frameToTransmit->peekAtFront<Ieee80211MacHeader>();
    if (context->getQoSContext()->ackPolicy->isBlockAckReqNeeded(context->getInProgressFrames(), context->getQoSContext()->txopProcedure))
        return 2;
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(macHeader))
        return 3;
    else {
        auto dataHeaderToTransmit = dynamicPtrCast<const Ieee80211DataHeader>(macHeader);
        OriginatorBlockAckAgreement *agreement = nullptr;
        if (context->getQoSContext()->blockAckAgreementHandler)
            agreement = context->getQoSContext()->blockAckAgreementHandler->getAgreement(dataHeaderToTransmit->getReceiverAddress(), dataHeaderToTransmit->getTid());
        auto ackPolicy = context->getQoSContext()->ackPolicy->computeAckPolicy(frameToTransmit, dataHeaderToTransmit, agreement);
        if (ackPolicy == AckPolicy::BLOCK_ACK)
            return 0;
        else if (ackPolicy == AckPolicy::NORMAL_ACK)
            return 1;
        else
            throw cRuntimeError("Unknown AckPolicy");
    }
}

bool TxOpFs::isRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    auto protectedFrame = context->getInProgressFrames()->getFrameToTransmit();
    return context->getRtsPolicy()->isRtsNeeded(protectedFrame, protectedFrame->peekAtFront<Ieee80211MacHeader>());
}

bool TxOpFs::isBlockAckReqRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return false; // FIXME QosRtsPolicy should handle this case
}

} // namespace ieee80211
} // namespace inet

