//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/recipient/RecipientQosAckPolicy.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientQosAckPolicy);

void RecipientQosAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IQosRateSelection *>(getModuleByPath(par("rateSelectionModule")));
    }
}

simtime_t RecipientQosAckPolicy::computeBasicBlockAckDuration(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const
{
    return rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq)->getDuration(LENGTH_BASIC_BLOCKACK);
}

simtime_t RecipientQosAckPolicy::computeAckDuration(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    return rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader)->getDuration(LENGTH_ACK);
}

//
// The cases when an ACK frame can be generated are shown in the frame exchange sequences listed in
// Annex G. On receipt of a management frame of subtype Action NoAck, a STA shall not send an ACK frame
// in response.
//
bool RecipientQosAckPolicy::isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    // TODO add mgmt frame NoAck check
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        if (dataHeader->getAckPolicy() != NORMAL_ACK)
            return false;
    return !header->getReceiverAddress().isMulticast();
}

//
// If the immediate Block Ack policy is used, the recipient shall respond to a
// Basic BlockAckReq frame with a Basic BlockAck frame.
// TODO
// If the delayed Block Ack policy is used, the recipient shall respond to a Basic BlockAckReq frame with an
// ACK frame. The recipient shall then send its Basic BlockAck response in a subsequently obtained TXOP.
// Once the contents of the Basic BlockAck frame have been prepared, the recipient shall send this frame in the
// earliest possible TXOP using the highest priority AC. The originator shall respond with an ACK frame upon
// receipt of the Basic BlockAck frame. If delayed Block Ack policy is used and if the HC is the recipient, then
// the HC may respond with a +CF-Ack frame if the Basic BlockAckReq frame is the final frame of the polled
// TXOP’s frame exchange. If delayed Block Ack policy is used and if the HC is the originator, then the HC may
// respond with a +CF-Ack frame if the Basic BlockAck frame is the final frame of the TXOP’s frame exchange.
//
bool RecipientQosAckPolicy::isBlockAckNeeded(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement) const
{
    if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        return agreement != nullptr;
        // TODO The Basic BlockAckReq frame shall be discarded if all MSDUs referenced by this
        // frame have been discarded from the transmit buffer due to expiry of their lifetime limit.
    }
    else
        throw cRuntimeError("Unsupported BlockAckReq");
}

//
// 8.2.5.7 Setting for control response frames
// For an ACK frame, the Duration/ID field is set to the value obtained from the Duration/ID field of the frame
// that elicited the response minus the time, in microseconds between the end of the PPDU carrying the frame
// that elicited the response and the end of the PPDU carrying the ACK frame.
//
simtime_t RecipientQosAckPolicy::computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    simtime_t duration = header->getDurationField() - modeSet->getSifsTime() - computeAckDuration(packet, header);
    return duration < 0 ? 0 : duration;
}

//
// For a BlockAck frame transmitted in response to a BlockAckReq frame or transmitted in response to a frame
// containing an implicit Block Ack request, the Duration/ID field is set to the value obtained from the
// Duration/ID field of the frame that elicited the response minus the time, in microseconds, between the end of
// the PPDU carrying the frame that elicited the response and the end of the PPDU carrying the BlockAck
// frame.
//
simtime_t RecipientQosAckPolicy::computeBasicBlockAckDurationField(Packet *packet, const Ptr<const Ieee80211BasicBlockAckReq>& basicBlockAckReq) const
{
    return basicBlockAckReq->getDurationField() - modeSet->getSifsTime() - computeBasicBlockAckDuration(packet, basicBlockAckReq);
}

} /* namespace ieee80211 */
} /* namespace inet */

