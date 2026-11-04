//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Contributor: Miguel González-López (Block Ack context and frame-sequence completion on transmit refusal)
//


#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

AmpduTransmitStep::AmpduTransmitStep(std::shared_ptr<PreparedDataTransmission> prepared, simtime_t ifs) :
    TransmitStep(nullptr, ifs), prepared(std::move(prepared))
{
}

Packet *AmpduTransmitStep::getFrameToTransmit()
{
    return prepared->envelope.get();
}

PreparedDataTransmission::~PreparedDataTransmission()
{
    if (rtsPrediction)
        drop(rtsPrediction.get());
    if (envelope)
        drop(envelope.get());
}

void PreparedDataTransmission::setEnvelope(Packet *packet)
{
    if (envelope)
        drop(envelope.get());
    envelope.reset(packet);
    if (envelope)
        take(envelope.get());
}

void PreparedDataTransmission::setRtsPrediction(Packet *packet)
{
    if (rtsPrediction)
        drop(rtsPrediction.get());
    rtsPrediction.reset(packet);
    if (rtsPrediction)
        take(rtsPrediction.get());
}

FrameSequenceContext::FrameSequenceContext(MacAddress address, Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, IRtsProcedure *rtsProcedure, IRtsPolicy *rtsPolicy, NonQoSContext *nonQoSContext, QoSContext *qosContext) :
    address(address),
    modeSet(modeSet),
    inProgressFrames(inProgressFrames),
    rtsProcedure(rtsProcedure),
    rtsPolicy(rtsPolicy),
    nonQoSContext(nonQoSContext),
    qosContext(qosContext)
{
}

void FrameSequenceContext::removeLastStep()
{
    ASSERT(!steps.empty());
    auto step = steps.back();
    steps.pop_back();
    delete step;
}

Packet *FrameSequenceContext::getDataFrameToTransmit() const
{
    return qosContext && qosContext->preparedData ? qosContext->preparedData->frame : inProgressFrames->getFrameToTransmit();
}

bool FrameSequenceContext::isNextFragment(const Packet *previous, const Packet *candidate)
{
    auto first = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(previous->peekAtFront<Ieee80211MacHeader>());
    auto next = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(candidate->peekAtFront<Ieee80211MacHeader>());
    if (!first || !next || !first->getMoreFragments() || first->getReceiverAddress().isMulticast() ||
        first->getType() != next->getType() || first->getReceiverAddress() != next->getReceiverAddress() ||
        first->getSequenceNumber() != next->getSequenceNumber() || first->getFragmentNumber() + 1 != next->getFragmentNumber())
        return false;
    if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(first)) {
        auto nextData = staticPtrCast<const Ieee80211DataHeader>(next);
        auto firstSequence = previous->findTag<Ieee80211DataSequenceTag>();
        auto nextSequence = candidate->findTag<Ieee80211DataSequenceTag>();
        // IEEE Std 802.11-2024, 10.4 and 10.23.2.9(a)(1): this is an MSDU fragment burst.
        return !data->getAMsduPresent() && !nextData->getAMsduPresent() && data->getTid() == nextData->getTid() &&
            firstSequence && nextSequence && firstSequence->getAllocationIndex() == nextSequence->getAllocationIndex();
    }
    return true;
}

bool FrameSequenceContext::isFragmentContinuation(const Packet *candidate, int numCompletedSteps) const
{
    if (!candidate || numCompletedSteps < 2 || !inProgressFrames->containsFrame(candidate))
        return false;
    auto ack = dynamic_cast<IReceiveStep *>(getStep(numCompletedSteps - 1));
    auto data = dynamic_cast<ITransmitStep *>(getStep(numCompletedSteps - 2));
    return ack && ack->getCompletion() == IFrameSequenceStep::Completion::ACCEPTED &&
        ack->getReceivedFrame()->peekAtFront<Ieee80211MacHeader>()->getType() == ST_ACK &&
        data && isNextFragment(data->getFrameToTransmit(), candidate);
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
        delete step;
    delete nonQoSContext;
    delete qosContext;
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
