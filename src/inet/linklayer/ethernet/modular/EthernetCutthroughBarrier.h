//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHBARRIER_H
#define __INET_ETHERNETCUTTHROUGHBARRIER_H

#include "inet/queueing/base/PacketDelayerBase.h"

namespace inet {

using namespace queueing;

class INET_API EthernetCutthroughBarrier : public PacketDelayerBase
{
  protected:
    virtual void processPacket(Packet *packet, simtime_t sendingTime) override;
    virtual clocktime_t computeDelay(Packet *packet) const override;
};

} // namespace inet

#endif

