//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"

namespace inet {
namespace ieee80211 {

const Ptr<Ieee80211RtsFrame> RtsProcedure::buildRtsFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    auto rtsFrame = makeShared<Ieee80211RtsFrame>(); // TODO "RTS");
    rtsFrame->setReceiverAddress(dataOrMgmtHeader->getReceiverAddress());
    return rtsFrame;
}

void RtsProcedure::processTransmittedRts(const Ptr<const Ieee80211RtsFrame>& rtsFrame)
{
    numSentRts++;
}

} /* namespace ieee80211 */
} /* namespace inet */

