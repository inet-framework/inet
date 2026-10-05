//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"

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
