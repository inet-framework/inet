//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVEWITHLIFETIME_H
#define __INET_RECEIVEWITHLIFETIME_H

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Drops packets whose age (now minus the creation time in their LifetimeHeader) exceeds
 * maxLifetime; otherwise pops the header and forwards. The time-based analogue of
 * ReceiveWithHopLimit.
 */
class INET_API ReceiveWithLifetime : public PacketFilterBase
{
  protected:
    simtime_t maxLifetime;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

