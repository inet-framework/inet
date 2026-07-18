//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDWITHLIFETIME_H
#define __INET_SENDWITHLIFETIME_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Stamps each packet with a LifetimeHeader recording its creation time, so a downstream
 * ReceiveWithLifetime can drop packets that have been in flight for too long. This is the
 * time-based analogue of the hop-count-based SendWithHopLimit.
 */
class INET_API SendWithLifetime : public PacketFlowBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

