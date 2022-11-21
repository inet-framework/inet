//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FULLPACKETSINK_H
#define __INET_FULLPACKETSINK_H

#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API FullPacketSink : public PassivePacketSinkBase, public virtual IPassivePacketSink
{
  public:
    virtual bool canPushSomePacket(cGate *gate) const override { return false; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return false; }

    virtual void pushPacket(Packet *packet, cGate *gate) override { return throw cRuntimeError("Packet sink is full"); }
};

} // namespace queueing
} // namespace inet

#endif

