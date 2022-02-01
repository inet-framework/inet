//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

void RecipientAckProcedure::processReceivedFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, IRecipientAckPolicy *ackPolicy, IProcedureCallback *callback)
{
    numReceivedAckableFrame++;
    // After a successful reception of a frame requiring acknowledgment, transmission of the ACK frame
    // shall commence after a SIFS period, without regard to the busy/idle state of the medium. (See Figure 9-9.)
    if (ackPolicy->isAckNeeded(dataOrMgmtHeader)) {
        auto ackFrame = buildAck(dataOrMgmtHeader);
        auto duration = ackPolicy->computeAckDurationField(packet, dataOrMgmtHeader);
        ackFrame->setDurationField(duration);
        auto ackPacket = new Packet("WlanAck", ackFrame);
        EV_DEBUG << "Duration for " << ackFrame->getName() << " is set to " << duration << " s.\n";
        callback->transmitControlResponseFrame(ackPacket, ackFrame, packet, dataOrMgmtHeader);
    }
}

void RecipientAckProcedure::processTransmittedAck(const Ptr<const Ieee80211AckFrame>& ack)
{
    numSentAck++;
}

const Ptr<Ieee80211AckFrame> RecipientAckProcedure::buildAck(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    auto ack = makeShared<Ieee80211AckFrame>();
    ack->setReceiverAddress(dataOrMgmtHeader->getTransmitterAddress());
    return ack;
}

} /* namespace ieee80211 */
} /* namespace inet */

