//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INPROGRESSQUEUE_H
#define __INET_INPROGRESSQUEUE_H

#include "inet/queueing/queue/PacketQueue.h"

namespace inet {
namespace queueing {

class INET_API InProgressQueue : public PacketQueue
{
  public:
    virtual bool canPushSomePacket(cGate *gate) const override { return false; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;
};

} // namespace queueing
} // namespace inet

#endif

