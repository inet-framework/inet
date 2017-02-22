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

#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "OriginatorProtectionMechanism.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorProtectionMechanism);

void OriginatorProtectionMechanism::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    }
}

//
// For all RTS frames sent by non-QoS STAs, the duration value is the time, in microseconds, required to
// transmit the pending data or management frame, plus one CTS frame, plus one ACK frame, plus three SIFS
// intervals. If the calculated duration includes a fractional microsecond, that value is rounded up to the next
// higher integer. For RTS frames sent by QoS STAs, see 8.2.5.
//
simtime_t OriginatorProtectionMechanism::computeRtsDurationField(Ieee80211RTSFrame* rtsFrame, Ieee80211DataOrMgmtFrame* pendingFrame)
{
    auto pendingFrameMode = rateSelection->computeMode(pendingFrame);
    RateSelection::setFrameMode(pendingFrame, pendingFrameMode); // FIXME: Kludge
    simtime_t pendingFrameDuration = pendingFrameMode->getDuration(pendingFrame->getBitLength());
    simtime_t ctsFrameDuration = rateSelection->computeResponseCtsFrameMode(rtsFrame)->getDuration(LENGTH_CTS);
    simtime_t ackFrameDuration = rateSelection->computeResponseAckFrameMode(pendingFrame)->getDuration(LENGTH_ACK);
    simtime_t durationId = ctsFrameDuration + pendingFrameDuration + ackFrameDuration;
    return durationId + 3 * modeSet->getSifsTime();
}

//
// Within all data frames sent during the CP by non-QoS STAs, the Duration/ID field is set according to the following rules:
//   — If the Address 1 field contains a group address, the duration value is set to 0.
//   — If the More Fragments bit is 0 in the Frame Control field of a frame and the Address 1 field contains
//     an individual address, the duration value is set to the time, in microseconds, required to transmit one
//     ACK frame, plus one SIFS interval.
//   — If the More Fragments bit is 1 in the Frame Control field of a frame and the Address 1 field contains
//     an individual address, the duration value is set to the time, in microseconds, required to transmit the
//     next fragment of this data frame, plus two ACK frames, plus three SIFS intervals.
//
simtime_t OriginatorProtectionMechanism::computeDataFrameDurationField(Ieee80211DataFrame* dataFrame, Ieee80211DataOrMgmtFrame* pendingFrame)
{
    simtime_t ackToDataFrameDuration = rateSelection->computeResponseAckFrameMode(dataFrame)->getDuration(LENGTH_ACK);
    if (dataFrame->getReceiverAddress().isMulticast())
        return 0;
    if (!dataFrame->getMoreFragments())
        return ackToDataFrameDuration + modeSet->getSifsTime();
    else {
        simtime_t pendingFrameDuration = rateSelection->computeMode(pendingFrame)->getDuration(pendingFrame->getBitLength());
        auto pendingFrameMode = rateSelection->computeMode(pendingFrame);
        RateSelection::setFrameMode(pendingFrame, pendingFrameMode);
        simtime_t ackToPendingFrame = pendingFrameMode->getDuration(LENGTH_ACK);
        return pendingFrameDuration + ackToDataFrameDuration + ackToPendingFrame + 3 * modeSet->getSifsTime();
    }
}

//
// Within all management frames sent during the CP by non-QoS STAs, the Duration field is set according to the following rules:
//   — If the DA field contains a group address, the duration value is set to 0.
//   — If the More Fragments bit is 0 in the Frame Control field of a frame and the DA field contains an
//     individual address, the duration value is set to the time, in microseconds, required to transmit one
//     ACK frame, plus one SIFS interval.
//   — If the More Fragments bit is 1 in the Frame Control field of a frame, and the DA field contains an
//     individual address, the duration value is set to the time, in microseconds, required to transmit the
//     next fragment of this management frame, plus two ACK frames, plus three SIFS intervals.
//
simtime_t OriginatorProtectionMechanism::computeMgmtFrameDurationField(Ieee80211ManagementFrame* mgmtFrame, Ieee80211DataOrMgmtFrame* pendingFrame)
{
    simtime_t ackFrameDuration = rateSelection->computeResponseAckFrameMode(mgmtFrame)->getDuration(LENGTH_ACK);
    if (mgmtFrame->getReceiverAddress().isMulticast())
        return 0;
    if (!mgmtFrame->getMoreFragments()) {
        return ackFrameDuration + modeSet->getSifsTime();
    }
    else {
        simtime_t pendingFrameDuration = rateSelection->computeMode(pendingFrame)->getDuration(pendingFrame->getBitLength());
        return pendingFrameDuration + 2 * ackFrameDuration + 3 * modeSet->getSifsTime();
    }
}

simtime_t OriginatorProtectionMechanism::computeDurationField(Ieee80211Frame* frame, Ieee80211DataOrMgmtFrame* pendingFrame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        return computeRtsDurationField(rtsFrame, pendingFrame);
    else if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        return computeDataFrameDurationField(dataFrame, pendingFrame);
    else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(frame))
        return computeMgmtFrameDurationField(mgmtFrame, pendingFrame);
    else
        throw cRuntimeError("Unknown frame");
}

} /* namespace ieee80211 */
} /* namespace inet */
