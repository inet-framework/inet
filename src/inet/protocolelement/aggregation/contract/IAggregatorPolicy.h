//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IAGGREGATORPOLICY_H
#define __INET_IAGGREGATORPOLICY_H

#include "inet/common/packet/Packet.h"

namespace inet {

class INET_API IAggregatorPolicy
{
  public:
    virtual bool isAggregatablePacket(Packet *aggregatedPacket, std::vector<Packet *>& aggregatedSubpackets, Packet *newSubpacket) = 0;
};

} // namespace inet

#endif

