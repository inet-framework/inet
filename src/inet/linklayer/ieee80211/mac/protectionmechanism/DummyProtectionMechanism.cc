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

#include "DummyProtectionMechanism.h"

namespace inet {
namespace ieee80211 {

Define_Module(DummyProtectionMechanism);

void DummyProtectionMechanism::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IQoSRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    }
}

simtime_t DummyProtectionMechanism::computeDurationField(Ieee80211Frame* frame, Ieee80211DataOrMgmtFrame* pendingFrame, TxopProcedure* txop, IRecipientQoSAckPolicy* ackPolicy)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame)) {
        return dataOrMgmtFrame->getReceiverAddress().isMulticast() ? 0 : (modeSet->getSifsTime() + rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame)->getDuration(LENGTH_ACK));
    }
    else if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame*>(frame)) {
        simtime_t duration =
                3 * modeSet->getSifsTime()
                + rateSelection->computeResponseCtsFrameMode(rtsFrame)->getDuration(LENGTH_CTS)
                + rateSelection->computeMode(frame, txop)->getDuration(pendingFrame->getBitLength())
                + rateSelection->computeResponseAckFrameMode(pendingFrame)->getDuration(LENGTH_ACK);

        return duration;
    }
    else
        throw cRuntimeError("Unknown frame");
}

} /* namespace ieee80211 */
} /* namespace inet */
