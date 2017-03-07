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

#include "RtsProcedure.h"

namespace inet {
namespace ieee80211 {

Ieee80211RTSFrame *RtsProcedure::buildRtsFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const
{
    Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame("RTS");
    rtsFrame->setReceiverAddress(dataOrMgmtFrame->getReceiverAddress());
    return rtsFrame;
}

void RtsProcedure::processTransmittedRts(Ieee80211RTSFrame* rtsFrame)
{
    numSentRts++;
}

} /* namespace ieee80211 */
} /* namespace inet */
