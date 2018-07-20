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

#include "CtsPolicy.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(CtsPolicy);

void CtsPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
    }
}

simtime_t CtsPolicy::computeCtsDuration(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const
{
    return rateSelection->computeResponseCtsFrameMode(rtsPacket, rtsFrame)->getDuration(LENGTH_CTS);
}

//
// The Duration field in the CTS frame shall be the duration field from the received RTS frame, adjusted by
// subtraction of aSIFSTime and the number of microseconds required to transmit the CTS frame at a data rate
// determined by the rules in 9.7.
//
simtime_t CtsPolicy::computeCtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const
{
    simtime_t duration = rtsFrame->getDuration() - modeSet->getSifsTime() - computeCtsDuration(rtsPacket, rtsFrame);
    return duration < 0 ? 0 : duration;
}

//
// A STA that is addressed by an RTS frame shall transmit a CTS frame after
// a SIFS period if the NAV at the STA receiving the RTS frame indicates that
// the medium is idle.
//
bool CtsPolicy::isCtsNeeded(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const
{
   return rx->isMediumFree();
}

} /* namespace ieee80211 */
} /* namespace inet */
