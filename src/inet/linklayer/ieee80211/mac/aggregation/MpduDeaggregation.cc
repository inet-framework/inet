//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/aggregation/MpduDeaggregation.h"

namespace inet {
namespace ieee80211 {

Register_Class(MpduDeaggregation);

bool MpduDeaggregation::visitSubframes(const Packet *packet, B offset, B length,
        const std::function<void(const Ptr<const Ieee80211MpduSubframeHeader>&, B, bool)>& visitDelimiter,
        const std::function<void(const Ptr<const Chunk>&, B)>& visitMpdu)
{
    if (offset < B(0) || length < B(0) || offset + length > packet->getDataLength())
        return false;
    if (length == B(0))
        return true;
    int flags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    Packet aggregate("mpdu-aggregate", packet->peekDataAt(offset, length, flags));
    // IEEE Std 802.11-2024, 9.7.1: HT uses a 12-bit MPDU length and four-octet subframe alignment.
    // The final HT subframe has no alignment padding.
    while (aggregate.getDataLength() > b(0)) {
        if (aggregate.getDataLength() < B(4))
            return false;
        B position = length - B(aggregate.getDataLength());
        const auto& delimiter = aggregate.popAtFront<Ieee80211MpduSubframeHeader>(B(4),
                Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED |
                Chunk::PF_ALLOW_REINTERPRETATION);
        bool valid = delimiter->isCorrect() && delimiter->isComplete() && delimiter->isProperlyRepresented() &&
                delimiter->getLength() >= 0 && delimiter->getLength() <= 4095 &&
                B(delimiter->getLength()) <= aggregate.getDataLength();
        visitDelimiter(delimiter, position, valid);
        if (!valid)
            return false;
        B memberLength(delimiter->getLength());
        if (memberLength == B(0))
            continue;
        const auto& mpdu = aggregate.popAtFront(memberLength, flags);
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
    return true;
}

std::vector<Packet *> *MpduDeaggregation::deaggregateFrame(Packet *aggregatedFrame)
{
    EV_DEBUG << "Deaggregating A-MPDU " << *aggregatedFrame << " into multiple packets.\n";
    std::vector<Packet *> *frames = new std::vector<Packet *>();
    cStringTokenizer tokenizer(aggregatedFrame->getName(), "+");
    bool complete = visitSubframes(aggregatedFrame, B(0), B(aggregatedFrame->getDataLength()),
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
    if (!complete) {
        for (auto frame : *frames) delete frame;
        frames->clear(); // A malformed suffix must not release a valid-looking prefix.
    }
    delete aggregatedFrame;
    EV_TRACE << "Created " << frames->size() << " packets from A-MPDU.\n";
    return frames;
}

} /* namespace ieee80211 */
} /* namespace inet */

