//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETPUSHTOSEND_H
#define __INET_PACKETPUSHTOSEND_H

#include "inet/queueing/base/PassivePacketSinkBase.h"

namespace inet {
namespace queueing {

class INET_API PacketPushToSend : public PassivePacketSinkBase
{
  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

