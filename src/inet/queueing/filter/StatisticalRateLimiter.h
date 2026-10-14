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
    double randomValue = NaN; // drawn before the decision that uses it, so that a query draws nothing

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    // the answer of the consumer, so that a source waits while the consumer
    // is full, also with a packet that the limiter would drop
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual double getDropProbability(const Packet *packet) const;
    virtual bool matchesPacket(const Packet *packet) const override;
    virtual void handlePacketFiltered(Packet *packet, bool matches) override;
};

} // namespace queueing
} // namespace inet

#endif

