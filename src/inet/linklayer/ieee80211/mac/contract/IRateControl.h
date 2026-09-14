//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRATECONTROL_H
#define __INET_IRATECONTROL_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
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

    // Returns the rate to use for a unicast frame addressed to the given receiver.
    virtual const physicallayer::IIeee80211Mode *getRate(const MacAddress& receiverAddress) = 0;
    // Legacy data/management attempt feedback. retryCount is the packet's data
    // recovery count; successful first data attempts report zero even after RTS failures.
    // Packets in all feedback methods are borrowed for the duration of the call.
    virtual void frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp) = 0;
    // Extended feedback: totalRetryCount is the current per-packet SRC + LRC,
    // including RTS failures, internal collisions, and the final failure on exhaustion.
    // Report exactly once per data attempt, before recovery clears the completed packet.
    virtual void frameTransmitted(Packet *frame, int retryCount, int totalRetryCount, bool isSuccessful, bool isGivenUp) = 0;
    // A failed RTS/CTS exchange, referring to the protected data/management packet.
    // Only isGivenUp marks a completed packet; successful CTS is not a completion.
    virtual void rtsFrameTransmissionFailed(Packet *frame, int totalRetryCount, bool isGivenUp) = 0;
    // Terminal retry-limit drop caused by an EDCA internal collision, not an on-air
    // attempt. Report once before cleanup; totalRetryCount is per-packet SRC + LRC,
    // including internal collisions and any preceding RTS/data failures.
    virtual void frameDroppedDueToInternalCollision(Packet *frame, int totalRetryCount) = 0;
    virtual void frameReceived(Packet *frame) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
