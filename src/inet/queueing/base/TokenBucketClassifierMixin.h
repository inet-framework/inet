//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

