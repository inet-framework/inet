//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H
#define __INET_IBLOCKACKAGREEMENTHANDLERCALLBACK_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"

namespace inet {

class Packet;

namespace ieee80211 {

class INET_API IBlockAckAgreementHandlerCallback
{
  public:
    virtual ~IBlockAckAgreementHandlerCallback() {}

    virtual void scheduleInactivityTimer(simtime_t timeout) = 0;
    virtual void scheduleAddbaResponseTimer(simtime_t deadline) = 0;
    virtual void cancelAddbaTransaction(uint64_t transactionId, Packet *excludedPacket) = 0;
    // Removes queued siblings of a sender-local DELBA without assuming that
    // the frame is still removable from the active frame sequence. The
    // agreement owner remains responsible for rejecting stale packets.
    virtual void cancelBlockAckTeardown(bool initiator, MacAddress peerAddress, Tid tid, uint64_t generationId, Packet *excludedPacket) {}
};

} // namespace ieee80211
} // namespace inet

#endif
