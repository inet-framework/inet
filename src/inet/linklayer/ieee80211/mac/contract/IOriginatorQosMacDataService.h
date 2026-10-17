//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IORIGINATORQOSMACDATASERVICE_H
#define __INET_IORIGINATORQOSMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorQosMacDataService : public IOriginatorMacDataService
{
  public:
    // Preview borrows the frame without extraction or sequence assignment.
    virtual Packet *peekEligibleFrame(queueing::IPacketQueue *queue) const = 0;
    virtual SequenceNumberCyclic getNextSequenceNumber(MacAddress peer, Tid tid) const = 0;
    virtual bool hasAMsduAggregation() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
