//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/TxOpFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/TxopExchangePlanner.h"
namespace inet {
namespace ieee80211 {
HcfFs::HcfFs() : RepeatingFs(new TxOpFs(), [this](RepeatingFs *, FrameSequenceContext *context) { return prepareExchange(context); }) {}

bool HcfFs::prepareExchange(FrameSequenceContext *context)
{
    auto qos = context->getQoSContext();
    TxopExchangePlanner planner;
    bool first = context->getNumSteps() == 0;
    auto plan = first ? planner.prepare(context, nullptr, false) : context->takeContinuationPlan();
    if (!plan)
        return false;
    if (plan->rateRevision != qos->mib->getRateSetRevision()) {
        context->setOutcome(FrameSequenceOutcome::RATE_STATE_CHANGED);
        return false;
    }
    auto decision = qos->txopProcedure->evaluate(*plan, simTime(), *qos->txnavEnd, !first);
    if (!decision.accepted) {
        qos->txopProcedure->commitAdmission(*plan, decision);
        if (first && decision.reason == TxopAdmissionReason::LIMIT_EXCEEDED)
            throw cRuntimeError("Initial exchange exceeds the positive TXOP limit; automatic airtime-based fragmentation is unsupported");
        return false;
    }
    context->setExchangePlan(std::move(plan));
    qos->txopProcedure->commitAdmission(*context->getExchangePlan(), decision);
    planner.prepareContinuation(context);
    return true;
}
} // namespace ieee80211
} // namespace inet
