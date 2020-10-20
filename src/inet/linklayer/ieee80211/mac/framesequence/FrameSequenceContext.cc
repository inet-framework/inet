//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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
}

simtime_t FrameSequenceContext::getIfs() const
{
    return getNumSteps() == 0 ? 0 : modeSet->getSifsTime(); // TODO: pifs
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
    // FIXME:
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
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
    inProgressFrames->clearDroppedFrames();
#endif
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

