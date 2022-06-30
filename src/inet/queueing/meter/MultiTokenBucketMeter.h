//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MULTITOKENBUCKETMETER_H
#define __INET_MULTITOKENBUCKETMETER_H

#include "inet/queueing/base/MultiTokenBucketMixin.h"
#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/base/TokenBucketMeterMixin.h"

namespace inet {
namespace queueing {

extern template class TokenBucketMeterMixin<MultiTokenBucketMixin<PacketMeterBase>>;

class INET_API MultiTokenBucketMeter : public TokenBucketMeterMixin<MultiTokenBucketMixin<PacketMeterBase>>
{
  protected:
    std::vector<std::string> labels;

  protected:
    virtual void initialize(int stage) override;

    virtual void meterPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

