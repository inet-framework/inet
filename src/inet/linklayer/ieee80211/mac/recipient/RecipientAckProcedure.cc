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
