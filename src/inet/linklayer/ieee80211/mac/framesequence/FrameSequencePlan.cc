// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequencePlan.h"

namespace inet::ieee80211 {

void FrameSequencePlan::append(std::unique_ptr<FrameSequencePlan> child)
{
    duration += child->duration;
    stepCount += child->stepCount;
    children.push_back(std::move(child));
}

std::vector<IFrameSequenceStep *> FrameSequencePlan::flatten() const
{
    std::vector<IFrameSequenceStep *> result;
    for (const auto& child : children) {
        auto childSteps = child->flatten();
        result.insert(result.end(), childSteps.begin(), childSteps.end());
    }
    for (const auto& step : steps)
        result.push_back(step.get());
    return result;
}

simtime_t FrameSequencePlan::remainingDuration(int offset, bool excludeIfs) const
{
    simtime_t result;
    bool first = true;
    for (auto step : flatten()) {
        if (auto tx = dynamic_cast<ITransmitStep *>(step)) {
            auto record = tx->getPreparedTransmit();
            if (record->offset >= offset) {
                result += record->airtime + (first && excludeIfs ? SIMTIME_ZERO : record->ifs);
                first = false;
            }
        }
        else {
            auto record = static_cast<IReceiveStep *>(step)->getPreparedReceive();
            if (record->offset >= offset) {
                result += record->airtime + record->ifs;
                first = false;
            }
        }
    }
    return result;
}

} // namespace inet::ieee80211
