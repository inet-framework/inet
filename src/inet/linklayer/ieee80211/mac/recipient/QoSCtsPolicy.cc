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

#include "inet/common/ModuleAccess.h"
#include "QoSCtsPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(QoSCtsPolicy);

void QoSCtsPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rateSelection = check_and_cast<IQoSRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    }
}

simtime_t QoSCtsPolicy::computeCtsDuration(Ieee80211RTSFrame *rtsFrame) const
{
    return rateSelection->computeResponseCtsFrameMode(rtsFrame)->getDuration(LENGTH_CTS);
}

//
// For a CTS frame that is not part of a dual CTS sequence transmitted in response to an RTS frame, the
// Duration/ID field is set to the value obtained from the Duration/ID field of the RTS frame that elicited the
// response minus the time, in microseconds, between the end of the PPDU carrying the RTS frame and the end
// of the PPDU carrying the CTS frame.
//
simtime_t QoSCtsPolicy::computeCtsDurationField(Ieee80211RTSFrame* rtsFrame) const
{
    simtime_t duration = rtsFrame->getDuration() - modeSet->getSifsTime() - computeCtsDuration(rtsFrame);
    return duration < 0 ? 0 : duration;
}

//
// A STA that is addressed by an RTS frame shall transmit a CTS frame after
// a SIFS period if the NAV at the STA receiving the RTS frame indicates that
// the medium is idle.
//
bool QoSCtsPolicy::isCtsNeeded(Ieee80211RTSFrame* rtsFrame) const
{
   return rx->isMediumFree();
}

} /* namespace ieee80211 */
} /* namespace inet */
