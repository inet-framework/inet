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

#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"

namespace inet {
namespace ieee80211 {

const Ptr<Ieee80211RtsFrame> RtsProcedure::buildRtsFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    auto rtsFrame = makeShared<Ieee80211RtsFrame>(); // TODO: "RTS");
    rtsFrame->setReceiverAddress(dataOrMgmtHeader->getReceiverAddress());
    return rtsFrame;
}

void RtsProcedure::processTransmittedRts(const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    numSentRts++;
}

} /* namespace ieee80211 */
} /* namespace inet */
