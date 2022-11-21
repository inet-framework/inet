//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICALRATELIMITER_H
#define __INET_STATISTICALRATELIMITER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

class INET_API StatisticalRateLimiter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    bps maxDatarate = bps(NaN);
    double maxPacketrate = NaN;

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    // NOTE: cannot answer because matchesPacket draws random numbers
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

