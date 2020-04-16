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

#include "inet/linklayer/ieee80211/mac/recipient/CtsProcedure.h"

namespace inet {
namespace ieee80211 {

void CtsProcedure::processReceivedRts(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ICtsPolicy *ctsPolicy, IProcedureCallback *callback)
{
    numReceivedRts++;
    // A STA that is addressed by an RTS frame shall transmit a CTS frame after a SIFS period
    // if the NAV at the STA receiving the RTS frame indicates that the medium is idle.
    if (ctsPolicy->isCtsNeeded(rtsFrame)) {
        auto ctsFrame = buildCts(rtsFrame);
        auto duration = ctsPolicy->computeCtsDurationField(rtsPacket, rtsFrame);
        ctsFrame->setDurationField(duration);
        auto ctsPacket = new Packet("CTS", ctsFrame);
        EV_DEBUG << "Duration for " << ctsPacket->getName() << " is set to " << duration << " s.\n";
        callback->transmitControlResponseFrame(ctsPacket, ctsFrame, rtsPacket, rtsFrame);
    }
    // If the NAV at the STA receiving the RTS indicates the medium is not idle,
    // that STA shall not respond to the RTS frame.
    else
        EV_WARN << "Ignoring received RTS according to CTS policy (probably the medium is not free).\n";
}

const Ptr<Ieee80211CtsFrame> CtsProcedure::buildCts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const
{
    const Ptr<Ieee80211CtsFrame>& cts = makeShared<Ieee80211CtsFrame>();
    // The RA field of the CTS frame shall be the value
    // obtained from the TA field of the to which this
    // CTS frame is a response.
    cts->setReceiverAddress(rtsFrame->getTransmitterAddress());
    return cts;
}

void CtsProcedure::processTransmittedCts(const Ptr<const Ieee80211CtsFrame>& ctsFrame)
{
    numSentCts++;
}

} /* namespace ieee80211 */
} /* namespace inet */
