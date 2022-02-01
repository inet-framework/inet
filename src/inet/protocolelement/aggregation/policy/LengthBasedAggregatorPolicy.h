//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LENGTHBASEDAGGREGATORPOLICY_H
#define __INET_LENGTHBASEDAGGREGATORPOLICY_H

#include "inet/common/Units.h"
#include "inet/protocolelement/aggregation/contract/IAggregatorPolicy.h"

namespace inet {

using namespace units::values;

class INET_API LengthBasedAggregatorPolicy : public cSimpleModule, public IAggregatorPolicy
{
  protected:
    int minNumSubpackets = -1;
    int maxNumSubpackets = -1;
    b minAggregatedLength = b(-1);
    b maxAggregatedLength = b(-1);

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool isAggregatablePacket(Packet *aggregatedPacket, std::vector<Packet *>& aggregatedSubpackets, Packet *newSubpacket) override;
};

} // namespace inet

#endif

