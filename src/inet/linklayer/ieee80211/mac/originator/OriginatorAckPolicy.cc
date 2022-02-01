//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/OriginatorAckPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorAckPolicy);

void OriginatorAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        ackTimeout = par("ackTimeout");
    }
}

bool OriginatorAckPolicy::isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header)) {
        return !dataOrMgmtHeader->getReceiverAddress().isMulticast(); // TODO + mgmt with NoAck check
    }
    return false;
}

//
// After transmitting an MPDU that requires an ACK frame as a response (see Annex G), the STA shall wait for an
// ACKTimeout interval, with a value of aSIFSTime + aSlotTime + aPHY-RX-START-Delay, starting at the
// PHY-TXEND.confirm primitive. If a PHY-RXSTART.indication primitive does not occur during the
// ACKTimeout interval, the STA concludes that the transmission of the MPDU has failed, and this STA shall
// invoke its backoff procedure upon expiration of the ACKTimeout interval.
//
simtime_t OriginatorAckPolicy::getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const
{
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseAckFrameMode(packet, header)->getPhyRxStartDelay() : ackTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */

