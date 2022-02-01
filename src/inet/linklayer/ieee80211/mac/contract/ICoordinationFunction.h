//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICOORDINATIONFUNCTION_H
#define __INET_ICOORDINATIONFUNCTION_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Interface for IEEE 802.11 Coordination Functions.
 */
class INET_API ICoordinationFunction
{
  public:
    virtual ~ICoordinationFunction() {}

    virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
    virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
    virtual void corruptedFrameReceived() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

