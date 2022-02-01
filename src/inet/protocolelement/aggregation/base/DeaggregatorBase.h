//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DEAGGREGATORBASE_H
#define __INET_DEAGGREGATORBASE_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API DeaggregatorBase : public PacketPusherBase
{
  protected:
    bool deleteSelf = false;

  protected:
    virtual void initialize(int stage) override;
    virtual std::vector<Packet *> deaggregatePacket(Packet *packet) = 0;

  public:
    virtual void pushPacket(Packet *aggregatedPacket, cGate *gate) override;
};

} // namespace inet

#endif

