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

#include "inet/linklayer/ieee80211/mac/lifetime/DcfTransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

void DcfTransmitLifetimeHandler::frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header)
{
    // don't care
}

//
// The source STA shall maintain a transmit MSDU timer for each MSDU being transmitted. The attribute
// dot11MaxTransmitMSDULifetime specifies the maximum amount of time allowed to transmit an MSDU. The
// timer starts on the initial attempt to transmit the first fragment of the MSDU.
//
void DcfTransmitLifetimeHandler::frameTransmitted(const Ptr<const Ieee80211DataHeader>& header)
{
    if (header->getFragmentNumber() == 0)
        lifetimes[header->getSequenceNumber().get()] = simTime();
}

//
// If the timer exceeds dot11MaxTransmitMSDULifetime, then all remaining fragments are discarded
// by the source STA and no attempt is made to complete transmission of the MSDU.
//
bool DcfTransmitLifetimeHandler::isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header)
{
    auto it = lifetimes.find(header->getSequenceNumber().get());
    if (it == lifetimes.end())
        throw cRuntimeError("There is no lifetime entry for frame = %s", header->getName());
    return (simTime() - it->second) >= maxTransmitLifetime;
}

} /* namespace ieee80211 */
} /* namespace inet */

