//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"

#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduDeaggregation);

static bool isRegionCorrect(const Ptr<const Chunk>& chunk, b offset, b length)
{
    if (chunk->Chunk::isIncorrect())
        return false;
    if (auto sequence = dynamicPtrCast<const SequenceChunk>(chunk)) {
        b position(0);
        for (const auto& child : sequence->getChunks()) {
            b end = position + child->getChunkLength();
            b first = std::max(offset, position);
            b last = std::min(offset + length, end);
            if (first < last && !isRegionCorrect(child, first - position, last - first))
                return false;
            position = end;
            if (position >= offset + length)
                break;
        }
        return true;
    }
    if (auto slice = dynamicPtrCast<const SliceChunk>(chunk))
        return isRegionCorrect(slice->getChunk(), slice->getOffset() + offset, length);
    return chunk->isCorrect();
}

bool MpduDeaggregation::visitSubframes(const Packet *packet, B offset, B length,
        const std::function<void(const Ptr<const Ieee80211MpduSubframeHeader>&, B, bool)>& visitDelimiter,
        const std::function<void(const Ptr<const Chunk>&, B)>& visitMpdu)
{
    if (offset < B(0) || length < B(0) || offset + length > packet->getDataLength())
        return false;
    if (length == B(0))
        return true;
    int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    // Byte conversion can omit source flags. Keep the original region as the error authority.
    const auto& original = packet->peekAll(flags);
    b originalOffset = packet->getFrontOffset() + offset;
    bool allCorrect = original->isCorrect();
    Packet aggregate("mpdu-aggregate", packet->peekDataAt(offset, length, flags));
    bool complete = true;
    // IEEE Std 802.11-2024, 9.7.1: HT uses a 12-bit MPDU length and four-octet subframe alignment.
    // The final HT subframe has no alignment padding.
    while (aggregate.getDataLength() > b(0)) {
        if (aggregate.getDataLength() < B(4))
            return false;
        B position = length - B(aggregate.getDataLength());
        // Recovery can enter a typed MPDU. Interpret its candidate delimiter bytes explicitly.
        const auto& delimiter = aggregate.popAtFront<Ieee80211MpduSubframeHeader>(B(4),
                Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED |
                Chunk::PF_ALLOW_REINTERPRETATION);
        bool valid = delimiter->isCorrect() && delimiter->isComplete() && delimiter->isProperlyRepresented() &&
                delimiter->getLength() >= 0 && delimiter->getLength() <= 4095 &&
                B(delimiter->getLength()) <= aggregate.getDataLength() &&
                (allCorrect || isRegionCorrect(original, originalOffset + position, B(4)));
        visitDelimiter(delimiter, position, valid);
        if (!valid) {
            // IEEE Std 802.11-2024, informative Annex O.2: examine candidates four octets apart.
            complete = false;
            continue;
        }
        B memberLength(delimiter->getLength());
        if (memberLength == B(0))
            continue;
        Ptr<const Chunk> mpdu = aggregate.popAtFront(memberLength, flags);
        if (mpdu->isCorrect() && !allCorrect &&
                !isRegionCorrect(original, originalOffset + position + B(4), memberLength)) {
            auto incorrect = mpdu->dupShared();
            incorrect->markIncorrect();
            mpdu = incorrect;
        }
        visitMpdu(mpdu, position + B(4));
        if (aggregate.getDataLength() == b(0))
            break;
        B padding((4 - memberLength.get<B>() % 4) % 4);
        if (padding > aggregate.getDataLength())
            return false;
        aggregate.setFrontOffset(aggregate.getFrontOffset() + padding);
        if (aggregate.getDataLength() == b(0))
            return false; // HT has no final alignment padding.
    }
    return complete;
}

std::vector<Packet *> *MpduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MPDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    // Extraction alone grants no MAC acceptance, including after delimiter recovery.
    // The caller must check delimiter validity, PHY outcomes, member FCS, and MAC conditions.
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    visitSubframes(aggregatedFrame, B(0), B(aggregatedFrame->getDataLength()),
            [](const auto&, B, bool) {},
            [&](const auto& mpdu, B offset) {
                auto frame = new Packet();
                frame->setName(tokenizer.hasMoreTokens() ? tokenizer.nextToken() : aggregatedFrame->getName());
                frame->insertAtBack(mpdu);
                frame->getRegionTags().copyTags(aggregatedFrame->getRegionTags(),
                        aggregatedFrame->getFrontOffset() + offset, frame->getFrontOffset(), mpdu->getChunkLength());
                EV_TRACE << "Created " << *frame << " from A-MPDU.\n";
                frames->push_back(frame);
            });
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MPDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

