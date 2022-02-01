//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDTOPORT_H
#define __INET_SENDTOPORT_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SendToPort : public PacketFlowBase
{
  protected:
    int port = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

