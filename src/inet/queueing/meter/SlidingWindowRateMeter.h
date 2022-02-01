//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SLIDINGWINDOWRATEMETER_H
#define __INET_SLIDINGWINDOWRATEMETER_H

#include "inet/queueing/base/PacketMeterBase.h"

namespace inet {
namespace queueing {

class INET_API SlidingWindowRateMeter : public PacketMeterBase
{
  protected:
    simtime_t timeWindow;
    int currentNumPackets = 0;
    b currentTotalPacketLength = b(0);
    bps datarate = bps(0);
    double packetrate = 0;

    std::map<simtime_t, b> packetLengths;

  protected:
    virtual void initialize(int stage) override;
    virtual void meterPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

