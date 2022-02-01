//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBUCKETMETER_H
#define __INET_TOKENBUCKETMETER_H

#include "inet/common/INETMath.h"
#include "inet/networklayer/diffserv/PacketMeterBase.h"

namespace inet {

/**
 * Simple token bucket meter.
 */
class INET_API TokenBucketMeter : public PacketMeterBase
{
  protected:
    double CIR = NaN; // Commited Information Rate (bits/sec)
    long CBS = 0; // Committed Burst Size (in bits)
    bool colorAwareMode = false;

    long Tc = 0; // token bucket for committed burst
    simtime_t lastUpdateTime;

    int numRcvd = 0;
    int numRed = 0;

  public:
    TokenBucketMeter() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void refreshDisplay() const override;

    virtual int meterPacket(Packet *packet) override;
};

} // namespace inet

#endif

