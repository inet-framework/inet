//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORMACDATASERVICE_H
#define __INET_IORIGINATORMACDATASERVICE_H

#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorMacDataService
{
  public:
    static simsignal_t packetFragmentedSignal;
    static simsignal_t packetAggregatedSignal;

  public:
    virtual ~IOriginatorMacDataService() {}

    virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

