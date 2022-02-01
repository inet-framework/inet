//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DUPLICATEREMOVAL_H
#define __INET_DUPLICATEREMOVAL_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API DuplicateRemoval : public PacketPusherBase
{
  protected:
    int lastSequenceNumber = -1;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

