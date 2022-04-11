//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REORDERING_H
#define __INET_REORDERING_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Reordering : public PacketPusherBase
{
  protected:
    int expectedSequenceNumber;
    std::map<int, Packet *> packets;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual ~Reordering();

    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

