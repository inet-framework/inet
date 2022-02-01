//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SINGLERATETHREECOLORMETER_H
#define __INET_SINGLERATETHREECOLORMETER_H

#include "inet/common/INETMath.h"
#include "inet/networklayer/diffserv/PacketMeterBase.h"

namespace inet {

/**
 * This class can be used as a meter in an ITrafficConditioner.
 * It marks the packets according to three parameters,
 * Committed Information Rate (CIR), Committed Burst Size (CBS),
 * and Excess Burst Size (EBS), to be either green, yellow or red.
 *
 * See RFC 2697.
 */
class INET_API SingleRateThreeColorMeter : public PacketMeterBase
{
  protected:
    double CIR = NaN; // Commited Information Rate (bits/sec)
    long CBS = 0; // Committed Burst Size (bits)
    long EBS = 0; // Excess Burst Size (bits)
    bool colorAwareMode = false;

    long Tc = 0; // token bucket for committed burst
    long Te = 0; // token bucket for excess burst
    simtime_t lastUpdateTime;

    int numRcvd = 0;
    int numYellow = 0;
    int numRed = 0;

  public:
    SingleRateThreeColorMeter() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual int meterPacket(Packet *packet) override;
};

} // namespace inet

#endif

