//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORMACDATASERVICE_H
#define __INET_IORIGINATORMACDATASERVICE_H

#include <functional>

#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorMacDataService
{
  public:
    using FrameEligibilityFunction = std::function<bool(const Packet *)>;

  public:
    static simsignal_t packetFragmentedSignal;
    static simsignal_t packetAggregatedSignal;

  public:
    virtual ~IOriginatorMacDataService() {}

    virtual void setFrameEligibilityFunction(const FrameEligibilityFunction& frameEligibilityFunction) = 0;
    virtual bool isFrameEligible(const Packet *packet) const = 0;
    virtual bool hasEligibleFrame(queueing::IPacketQueue *pendingQueue) const = 0;
    virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
