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

#include "OriginatorAckPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorAckPolicy);

void OriginatorAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
        ackTimeout = par("ackTimeout");
    }
}

bool OriginatorAckPolicy::isAckNeeded(Ieee80211DataOrMgmtFrame* frame) const
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame)) {
        return !dataOrMgmtFrame->getReceiverAddress().isMulticast(); // TODO: + mgmt with NoAck check
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
simtime_t OriginatorAckPolicy::getAckTimeout(Ieee80211DataOrMgmtFrame *frame) const
{
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseAckFrameMode(frame)->getPhyRxStartDelay() : ackTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */
