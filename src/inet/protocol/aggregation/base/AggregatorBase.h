//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_AGGREGATORBASE_H
#define __INET_AGGREGATORBASE_H

#include "inet/protocol/aggregation/contract/IAggregatorPolicy.h"
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

#endif // ifndef __INET_AGGREGATORBASE_H

