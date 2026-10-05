//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequencePlanningContext.h"
#include "inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

FrameSequenceContext::FrameSequenceContext(MacAddress address, Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, IRtsProcedure *rtsProcedure, IRtsPolicy *rtsPolicy, NonQoSContext *nonQoSContext, QoSContext *qosContext) :
    address(address),
    modeSet(modeSet),
    inProgressFrames(inProgressFrames),
    rtsProcedure(rtsProcedure),
    rtsPolicy(rtsPolicy),
    nonQoSContext(nonQoSContext),
    qosContext(qosContext)
{
    if (inProgressFrames)
        inProgressFrames->retainFrameReferences();
}

simtime_t FrameSequenceContext::getIfs() const
{
    return getNumSteps() == 0 ? 0 : modeSet->getSifsTime(); // TODO pifs
}

simtime_t FrameSequenceContext::getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtframe) const
{
    return qosContext ? qosContext->ackPolicy->getAckTimeout(packet, dataOrMgmtframe) : nonQoSContext->ackPolicy->getAckTimeout(packet, dataOrMgmtframe);
}

simtime_t FrameSequenceContext::getCtsTimeout(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const
{
    return rtsPolicy->getCtsTimeout(packet, rtsFrame);
}

bool FrameSequenceContext::isForUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    return header->getReceiverAddress() == address || (header->getReceiverAddress().isMulticast() && !isSentByUs(header));
}

bool FrameSequenceContext::isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    // FIXME
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == address;
    else
        return false;
}

FrameSequenceContext::~FrameSequenceContext()
{
    for (auto step : steps)
        if ((step->getType() == IFrameSequenceStep::Type::TRANSMIT && !static_cast<ITransmitStep *>(step)->getPreparedTransmit()) ||
            (step->getType() == IFrameSequenceStep::Type::RECEIVE && !static_cast<IReceiveStep *>(step)->getPreparedReceive()))
            delete step;
    delete nonQoSContext;
    delete qosContext;
    if (inProgressFrames)
        inProgressFrames->releaseFrameReferences();
}

AckPolicy FrameSequenceContext::getAckPolicy()
{
    auto packet = getFrameToTransmit();
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    OriginatorBlockAckAgreement *agreement = nullptr;
    if (qosContext->blockAckAgreementHandler)
        agreement = qosContext->blockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid());
    return qosContext->ackPolicy->computeAckPolicy(packet, header, agreement);
}

bool FrameSequenceContext::isBlockAckReqNeeded()
{
    return qosContext->ackPolicy->isBlockAckReqNeeded(inProgressFrames, qosContext->txopProcedure);
}

bool FrameSequenceContext::prepareInitialExchange(IFrameSequence *sequence)
{
    inProgressFrames->stageForPlanning();
    FrameSequencePlanningContext planning(*this, planningRateSelection, false);
    auto result = sequence->planSequence(planning);
    if (result.status == FrameSequencePlanResult::Status::EMPTY)
        return false;
    if (result.status != FrameSequencePlanResult::Status::READY || result.plan->stepCount == 0)
        throw cRuntimeError("The selected HCF exchange does not support preparation");
    activePlan = std::move(result.plan);
    auto txop = qosContext->txopProcedure;
    auto admission = txop->admitExchange(activePlan->duration, activePlan->candidate, simTime());
    if (admission == TxopProcedure::Admission::REFUSED) {
        const auto& candidate = activePlan->candidate;
        std::ostringstream detail;
        detail << " " << inProgressFrames->getFragmentationDescription();
        detail << " protection=" << (activePlan->stepCount > 2 ? "RTS/CTS" : "none");
        for (auto step : activePlan->flatten()) {
            if (auto tx = dynamic_cast<ITransmitStep *>(step)) {
                auto record = tx->getPreparedTransmit();
                detail << " tx=" << record->frame->getName() << " length=" << record->length << " mode=" << record->mode->getName()
                       << " ackPolicy=" << record->ackPolicy << " airtime=" << record->airtime;
            }
            else {
                auto record = static_cast<IReceiveStep *>(step)->getPreparedReceive();
                detail << " response=" << record->length << " mode=" << record->mode->getName() << " cost=" << record->ifs + record->airtime;
            }
        }
        throw cRuntimeError("INET model limit: initial exchange exceeds TXOP; peer=%s tid=%d sequence=%d fragment=%d limit=%s cost=%s available=%s transmissions=%d fragments=%d earlierRetransmission=%d; airtime-driven fragmentation is unsupported;%s",
            candidate.receiver.str().c_str(), candidate.tid, candidate.sequenceControl.getSequenceNumber(), candidate.sequenceControl.getFragmentNumber(),
            txop->getLimit().str().c_str(), activePlan->duration.str().c_str(), txop->getRemaining().str().c_str(),
            candidate.transmissions, candidate.fragmentCount, candidate.earlierFragmentRetransmitted, detail.str().c_str());
    }
    activePlan->overrunAllowed = admission != TxopProcedure::Admission::FIT;
    prepareNextExchange(sequence);
    sequence->startPlannedSequence(this, 0, *activePlan);
    return true;
}

void FrameSequenceContext::prepareNextExchange(IFrameSequence *sequence)
{
    inProgressFrames->stageForPlanning();
    FrameSequencePlanningContext planning(*this, planningRateSelection, activePlan->continuation);
    if (!planning.getCandidate() || planning.getCandidate()->identity != activePlan->candidate.identity)
        throw cRuntimeError("Active exchange lost its staged candidate");
    planning.projectCompletion();
    nextPlan.reset();
    // Preserve the existing group-traffic path: one group MPDU per channel grant.
    if (planning.hasFrameToTransmit() && !activePlan->candidate.receiver.isMulticast() && !planning.getCandidate()->receiver.isMulticast()) {
        auto result = sequence->planSequence(planning);
        if (result.status == FrameSequencePlanResult::Status::UNSUPPORTED)
            throw cRuntimeError("The selected next HCF exchange does not support preparation");
        if (result.status == FrameSequencePlanResult::Status::READY && result.plan->stepCount > 0) {
            auto txop = qosContext->txopProcedure;
            auto admission = txop->admitExchange(result.plan->duration, result.plan->candidate,
                simTime() + activePlan->duration, &activePlan->candidate);
            if (admission != TxopProcedure::Admission::REFUSED) {
                result.plan->overrunAllowed = admission != TxopProcedure::Admission::FIT;
                nextPlan = std::move(result.plan);
            }
        }
    }
    for (auto step : activePlan->flatten())
        if (auto tx = dynamic_cast<TransmitStep *>(step))
            tx->setPreparedDuration(SingleProtectionMechanism::computePreparedDurationField(*tx->getPreparedTransmit(), *activePlan, nextPlan.get()));
}

bool FrameSequenceContext::advanceExchange(IFrameSequence *sequence)
{
    if (!planValid || !nextPlan || !inProgressFrames->isRetained(nextPlan->candidate))
        return false;
    auto actual = inProgressFrames->inspectStagedFrames();
    auto candidate = std::find_if(actual.begin(), actual.end(), [](const StagedFrameView& view) { return view.ack.eligible; });
    if (candidate == actual.end() || candidate->identity != nextPlan->candidate.identity ||
        candidate->ack.phase != nextPlan->candidate.ack.phase || candidate->transmissions != nextPlan->candidate.transmissions)
        return false;
    auto txop = qosContext->txopProcedure;
    if (txop->admitExchange(nextPlan->duration, *candidate, simTime()) == TxopProcedure::Admission::REFUSED ||
        !txop->fitsTxnav(nextPlan->remainingDuration(0, true), simTime()))
        return false;
    retiredPlans.push_back(std::move(activePlan));
    activePlan = std::move(nextPlan);
    activeFirstStep = steps.size();
    prepareNextExchange(sequence);
    sequence->startPlannedSequence(this, activeFirstStep, *activePlan);
    return true;
}

bool FrameSequenceContext::isPreparedTransmissionPermitted(const PreparedTransmit& record) const
{
    if (!planValid || !activePlan || !inProgressFrames->isRetained(activePlan->candidate) ||
        record.frame->getDataLength() != record.length || !modeSet->containsMode(record.mode))
        return false;
    auto actual = inProgressFrames->inspectStagedFrames();
    auto candidate = std::find_if(actual.begin(), actual.end(), [this](const StagedFrameView& view) { return view.identity == activePlan->candidate.identity; });
    if (candidate == actual.end() || candidate->ack.phase != activePlan->candidate.ack.phase ||
        candidate->transmissions != activePlan->candidate.transmissions)
        return false;
    for (auto step : activePlan->flatten()) {
        auto receive = dynamic_cast<IReceiveStep *>(step);
        if (receive && !modeSet->containsMode(receive->getPreparedReceive()->mode))
            return false;
    }
    auto cost = activePlan->remainingDuration(record.offset, true);
    return activePlan->overrunAllowed || cost <= qosContext->txopProcedure->getRemaining();
}

Register_ResultFilter("frameSequenceDuration", FrameSequenceDurationFilter);

void FrameSequenceDurationFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    fire(this, t, check_and_cast<FrameSequenceContext *>(object)->getDuration(), details);
}

Register_ResultFilter("frameSequenceNumPackets", FrameSequenceNumPacketsFilter);

void FrameSequenceNumPacketsFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    fire(this, t, (intval_t)check_and_cast<FrameSequenceContext *>(object)->getNumSteps(), details);
}

} // namespace ieee80211
} // namespace inet
