//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TWORATETHREECOLORMETER_H
#define __INET_TWORATETHREECOLORMETER_H

#include "inet/common/INETMath.h"
#include "inet/networklayer/diffserv/PacketMeterBase.h"

namespace inet {

/**
 * This class can be used as a meter in an ITrafficConditioner.
 * It marks the packets based on two rates, Peak Information Rate (PIR)
 * and Committed Information Rate (CIR), and their associated burst sizes
 * to be either green, yellow or red.
 *
 * See RFC 2698.
 */
class INET_API TwoRateThreeColorMeter : public PacketMeterBase
{
  protected:
    double PIR = NaN; // Peak Information Rate (bits/sec)
    long PBS = 0; // Peak Burst Size (bits)
    double CIR = NaN; // Commited Information Rate (bits/sec)
    long CBS = 0; // Committed Burst Size (bits)
    bool colorAwareMode = false;

    long Tp = 0; // token bucket for peak burst
    long Tc = 0; // token bucket for committed burst
    simtime_t lastUpdateTime;

    int numRcvd = 0;
    int numYellow = 0;
    int numRed = 0;

  public:
    TwoRateThreeColorMeter() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void refreshDisplay() const override;

    virtual int meterPacket(Packet *packet) override;
};

} // namespace inet

#endif

