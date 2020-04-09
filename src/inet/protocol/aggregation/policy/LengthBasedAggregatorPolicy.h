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

#ifndef __INET_LENGTHBASEDAGGREGATORPOLICY_H
#define __INET_LENGTHBASEDAGGREGATORPOLICY_H

#include "inet/common/Units.h"
#include "inet/protocol/aggregation/contract/IAggregatorPolicy.h"

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

#endif // ifndef __INET_LENGTHBASEDAGGREGATORPOLICY_H

