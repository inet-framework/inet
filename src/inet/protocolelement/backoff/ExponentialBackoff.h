//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EXPONENTIALBACKOFF_H
#define __INET_EXPONENTIALBACKOFF_H

#include "inet/queueing/base/PacketDelayerBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Delays each packet by a random backoff whose window doubles with the (re)transmission
 * attempt number carried in the packet's TransmissionAttemptReq tag -- the truncated binary
 * exponential backoff of CSMA/collision-avoidance MACs.
 *
 * For attempt a the backoff exponent is E = min(minBackoffExponent + a, maxBackoffExponent),
 * the contention window is CW = 2^E - 1, and the delay is a uniformly random number of slots
 * in [0, CW] times slotTime. Unlike a plain PacketDelayer with a random delay parameter, the
 * window grows with the attempt, which is what a fixed distribution cannot express.
 */
class INET_API ExponentialBackoff : public PacketDelayerBase
{
  protected:
    simtime_t slotTime;
    int minBackoffExponent = 0;
    int maxBackoffExponent = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual clocktime_t computeDelay(Packet *packet) const override;
};

} // namespace inet

#endif

