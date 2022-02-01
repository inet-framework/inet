//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRATECONTROL_H
#define __INET_IRATECONTROL_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for auto rate control algorithms. Examples of rate
 * control algorithms are ARF, AARF, Onoe and Minstrel.
 */
class INET_API IRateControl
{
  public:
    virtual ~IRateControl() {}

    virtual const physicallayer::IIeee80211Mode *getRate() = 0;
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) = 0;
    virtual void frameReceived(Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

