//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

