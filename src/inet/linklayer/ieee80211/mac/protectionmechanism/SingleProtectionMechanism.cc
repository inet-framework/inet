//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.h"
namespace inet {
namespace ieee80211 {
Define_Module(SingleProtectionMechanism);
simtime_t SingleProtectionMechanism::computeDurationField(const ITransmitStep *step,
        const TxopExchangePlan& plan, const TxopExchangePlan *continuation) const
{
    // IEEE Std 802.11-2024, 9.2.5.2: reserve only prepared exchanges.
    auto it = std::find(plan.steps.begin(), plan.steps.end(), step);
    if (it == plan.steps.end())
        throw cRuntimeError("Transmit step does not belong to the admitted exchange");
    simtime_t duration;
    for (++it; it != plan.steps.end(); ++it)
        duration += (*it)->getPreparedInterval() + (*it)->getPpduDuration();
    bool payloadStep = false;
    for (auto candidate : plan.steps)
        if (candidate == step)
            payloadStep = dynamic_cast<ITransmitStep *>(candidate)->getFrameToTransmit() == plan.payload;
    if (payloadStep && continuation)
        duration += continuation->getDuration();
    // 9.2.5.1: round a fractional microsecond upward before serialization.
    auto micros = duration.inUnit(SIMTIME_US);
    if (SimTime(micros, SIMTIME_US) < duration)
        ++micros;
    if (micros > 32767)
        throw cRuntimeError("Prepared reservation exceeds the Duration/ID range");
    return SimTime(std::max(int64_t(0), micros), SIMTIME_US);
}
} // namespace ieee80211
} // namespace inet
