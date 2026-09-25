//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/mac/framesequence/TxOpFs.h"
namespace inet {
namespace ieee80211 {
void TxOpFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    step = 0;
}
IFrameSequenceStep *TxOpFs::prepareStep(FrameSequenceContext *context)
{
    const auto& steps = context->getExchangePlan()->steps;
    return step < steps.size() ? steps[step] : nullptr;
}
bool TxOpFs::completeStep(FrameSequenceContext *context)
{
    auto current = context->getExchangePlan()->steps.at(step++);
    if (auto receive = dynamic_cast<IReceiveStep *>(current)) {
        auto packet = receive->getReceivedFrame();
        if (!packet)
            return false;
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        if (!context->isForUs(header) || header->getType() != receive->getExpectedFrameType())
            return false;
        auto twoAddress = dynamicPtrCast<const Ieee80211TwoAddressHeader>(header);
        return !twoAddress || twoAddress->getTransmitterAddress() == receive->getExpectedPeer();
    }
    return true;
}
std::string TxOpFs::getHistory() const
{
    return "prepared-exchange:" + std::to_string(step);
}
} // namespace ieee80211
} // namespace inet
