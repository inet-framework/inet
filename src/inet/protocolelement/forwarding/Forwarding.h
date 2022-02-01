//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FORWARDING_H
#define __INET_FORWARDING_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Forwarding : public PacketPusherBase
{
  protected:
    L3Address address;

  protected:
    virtual void initialize(int stage) override;

    virtual std::pair<L3Address, int> findNextHop(const L3Address& destinationAddress);

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

