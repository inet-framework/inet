//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_TOKENBUCKETCLASSIFIERMIXIN_H
#define __INET_TOKENBUCKETCLASSIFIERMIXIN_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

template<typename T>
class INET_API TokenBucketClassifierMixin : public T
{
  protected:
    double tokenConsumptionPerPacket = NaN;
    double tokenConsumptionPerBit = NaN;

  protected:
    virtual void initialize(int stage) override
    {
        T::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            tokenConsumptionPerPacket = T::par("tokenConsumptionPerPacket");
            tokenConsumptionPerBit = T::par("tokenConsumptionPerBit");
        }
    }

    virtual double getNumPacketTokens(Packet *packet) const
    {
        return b(packet->getDataLength()).get() * tokenConsumptionPerBit + tokenConsumptionPerPacket;
    }
};

} // namespace queueing
} // namespace inet

#endif

