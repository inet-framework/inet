//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBUCKETMETER_H
#define __INET_TOKENBUCKETMETER_H

#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/base/TokenBucketMeterMixin.h"
#include "inet/queueing/base/TokenBucketMixin.h"

namespace inet {
namespace queueing {

extern template class TokenBucketMeterMixin<TokenBucketMixin<PacketMeterBase>>;

class INET_API TokenBucketMeter : public TokenBucketMeterMixin<TokenBucketMixin<PacketMeterBase>>
{
  protected:
    const char *label = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void meterPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

