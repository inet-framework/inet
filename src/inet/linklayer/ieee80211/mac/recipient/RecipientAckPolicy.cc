//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckPolicy.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientAckPolicy);

void RecipientAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
    }
}

simtime_t RecipientAckPolicy::computeAckDuration(Packet *dataOrMgmtPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    return rateSelection->computeResponseAckFrameMode(dataOrMgmtPacket, dataOrMgmtHeader)->getDuration(LENGTH_ACK);
}

//
// The cases when an ACK frame can be generated are shown in the frame exchange sequences listed in
// Annex G. On receipt of a management frame of subtype Action NoAck, a STA shall not send an ACK frame
// in response.
//
bool RecipientAckPolicy::isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    // TODO add mgmt NoAck check
    return !header->getReceiverAddress().isMulticast();
}

//
// 8.3.1.4 ACK frame format
//
// For ACK frames sent by non-QoS STAs, if the More Fragments bit was equal to 0 in the Frame Control field
// of the immediately previous individually addressed data or management frame, the duration value is set to 0.
// In other ACK frames sent by non-QoS STAs, the duration value is the value obtained from the Duration/ID
// field of the immediately previous data, management, PS-Poll, BlockAckReq, or BlockAck*** frame minus the
// time, in microseconds, required to transmit the ACK frame and its SIFS interval. If the calculated duration
// includes a fractional microsecond, that value is rounded up to the next higher integer.
//
// NOTE: ** BlockAckReq, BlockAck to a NonQoS STA???
//
simtime_t RecipientAckPolicy::computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    if (header->getMoreFragments()) {
        auto duration = header->getDurationField() - modeSet->getSifsTime() - computeAckDuration(packet, header);
        duration = ceil(duration, SimTime(1, SIMTIME_US));
        if (duration < 0)
            EV_WARN << "ACK duration field would be negative, returning 0 instead.\n";
        return duration < 0 ? 0 : duration;
    }
    return 0;
}

} /* namespace ieee80211 */
} /* namespace inet */

