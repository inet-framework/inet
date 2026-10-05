// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequencePlanningContext.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet::ieee80211 {

FrameSequencePlanningContext::FrameSequencePlanningContext(const FrameSequenceContext& source, IQosRateSelection *rateSelection, bool continuation) :
    FrameSequenceContext(source.getAddress(), source.getModeSet(), source.getInProgressFrames(), source.getRtsProcedure(), source.getRtsPolicy(),
        source.getNonQoSContext() ? new NonQoSContext(*source.getNonQoSContext()) : nullptr,
        source.getQoSContext() ? new QoSContext(*source.getQoSContext()) : nullptr),
    frames(source.getInProgressFrames()->inspectStagedFrames()), continuation(continuation), rateSelection(rateSelection)
{
    while (position < frames.size() && !frames[position].ack.eligible)
        position++;
}

const StagedFrameView *FrameSequencePlanningContext::getCandidate() const
{
    return position < frames.size() ? &frames[position] : nullptr;
}

Packet *FrameSequencePlanningContext::getFrameToTransmit() const
{
    auto candidate = getCandidate();
    return candidate ? const_cast<Packet *>(candidate->frame) : nullptr;
}

bool FrameSequencePlanningContext::hasFrameToTransmit() const
{
    return getCandidate() != nullptr;
}

simtime_t FrameSequencePlanningContext::getIfs() const
{
    return continuation || offset != 0 ? modeSet->getSifsTime() : SIMTIME_ZERO;
}

AckPolicy FrameSequencePlanningContext::getAckPolicy()
{
    if (!ackPolicySelected) {
        auto packet = getFrameToTransmit();
        auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(header))
            selectedAckPolicy = qosContext->ackPolicy->computeAckPolicy(packet, data, nullptr);
        ackPolicySelected = true;
    }
    return selectedAckPolicy;
}

void FrameSequencePlanningContext::projectCompletion()
{
    if (position >= frames.size())
        throw cRuntimeError("Cannot project completion without a staged candidate");
    // This is private calculation state. No protocol success callback occurs here.
    frames[position].ack.eligible = false;
    frames[position].ack.outstanding = false;
    frames[position].ack.phase = AckFrameState::Phase::NORMAL_ACK_ARRIVED;
    position++;
    while (position < frames.size() && !frames[position].ack.eligible)
        position++;
    offset = 0;
    continuation = true;
    ackPolicySelected = false;
    lastTransmit = nullptr;
}

std::unique_ptr<FrameSequencePlan> FrameSequencePlanningContext::makePlan(const IFrameSequence *sequence) const
{
    auto plan = std::make_unique<FrameSequencePlan>();
    plan->sequence = sequence;
    plan->offset = offset;
    plan->continuation = continuation;
    if (auto candidate = getCandidate())
        plan->candidate = *candidate;
    return plan;
}

void FrameSequencePlanningContext::addTransmit(FrameSequencePlan& plan, bool rts)
{
    auto packet = getFrameToTransmit();
    if (!packet)
        throw cRuntimeError("Prepared transmit requires a staged candidate");
    std::unique_ptr<TransmitStep> step;
    if (rts) {
        auto header = rtsProcedure->buildRtsFrame(packet->peekAtFront<Ieee80211DataOrMgmtHeader>());
        auto control = new Packet("RTS", header);
        control->insertAtBack(makeShared<Ieee80211MacTrailer>());
        step = std::make_unique<RtsTransmitStep>(packet, control, getIfs());
    }
    else
        step = std::make_unique<TransmitStep>(packet, getIfs());
    PreparedTransmit record;
    record.frame = step->getFrameToTransmit();
    record.length = record.frame->getDataLength();
    record.ifs = getIfs();
    record.ackPolicy = getAckPolicy();
    std::unique_ptr<Packet> view(record.frame->dup());
    record.mode = rateSelection->computeMode(view.get(), view->peekAtFront<Ieee80211MacHeader>(), qosContext->txopProcedure);
    if (!record.mode)
        throw cRuntimeError("Rate policy returned no prepared mode");
    record.airtime = record.mode->getDuration(record.length);
    record.offset = offset++;
    step->setPreparedTransmit(record);
    plan.duration += record.ifs + record.airtime;
    plan.stepCount++;
    lastTransmit = step.get();
    plan.steps.push_back(std::move(step));
}

void FrameSequencePlanningContext::addReceive(FrameSequencePlan& plan, bool cts)
{
    if (!lastTransmit)
        throw cRuntimeError("Prepared response has no request");
    auto request = lastTransmit->getPreparedTransmit();
    std::unique_ptr<Packet> view(request->frame->dup());
    view->addTagIfAbsent<physicallayer::Ieee80211ModeReq>()->setMode(request->mode);
    PreparedReceive record;
    record.request = request->frame;
    record.ifs = modeSet->getSifsTime();
    simtime_t timeout;
    if (cts) {
        auto header = view->peekAtFront<Ieee80211RtsFrame>();
        record.mode = rateSelection->computeResponseCtsFrameMode(view.get(), header);
        record.length = makeShared<Ieee80211CtsFrame>()->getChunkLength() + B(4);
        timeout = rtsPolicy->getCtsTimeoutForMode(record.mode);
    }
    else {
        auto header = view->peekAtFront<Ieee80211DataOrMgmtHeader>();
        record.mode = rateSelection->computeResponseAckFrameMode(view.get(), header);
        record.length = makeShared<Ieee80211AckFrame>()->getChunkLength() + B(4);
        timeout = qosContext->ackPolicy->getAckTimeoutForMode(record.mode);
    }
    record.airtime = record.mode->getDuration(record.length);
    record.offset = offset++;
    auto step = std::make_unique<ReceiveStep>(timeout);
    step->setPreparedReceive(record);
    plan.duration += record.ifs + record.airtime;
    plan.stepCount++;
    plan.steps.push_back(std::move(step));
}

} // namespace inet::ieee80211
