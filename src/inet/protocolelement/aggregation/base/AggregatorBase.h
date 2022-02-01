//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AGGREGATORBASE_H
#define __INET_AGGREGATORBASE_H

#include "inet/protocolelement/aggregation/contract/IAggregatorPolicy.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API AggregatorBase : public PacketPusherBase
{
  protected:
    bool deleteSelf = false;
    IAggregatorPolicy *aggregatorPolicy = nullptr;

    std::vector<Packet *> aggregatedSubpackets;
    Packet *aggregatedPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual IAggregatorPolicy *createAggregatorPolicy(const char *aggregatorPolicyClass) const;

    virtual bool isAggregating() { return aggregatedPacket != nullptr; }
    virtual void startAggregation(Packet *packet);
    virtual void continueAggregation(Packet *packet);
    virtual void endAggregation(Packet *packet);

  public:
    virtual ~AggregatorBase() { delete aggregatedPacket; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

