//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDFROMPORT_H
#define __INET_SENDFROMPORT_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Prepends a SourcePortHeader carrying the configured source port. The source-port
 * counterpart of SendToPort; together with SendFromL3Address it lets a receiver
 * reconstruct who to reply to.
 */
class INET_API SendFromPort : public PacketFlowBase
{
  protected:
    int port = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

