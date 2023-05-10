//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSINKREF_H
#define __INET_PASSIVEPACKETSINKREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSinkRef : public ModuleRefByGate<IPassivePacketSink>
{
  public:
    virtual bool canPushSomePacket() const {
        return referencedModule->canPushSomePacket(referencedGate);
    }

    virtual bool canPushPacket(Packet *packet) const {
        return referencedModule->canPushPacket(packet, referencedGate);
    }

    virtual void pushPacket(Packet *packet) {
        referencedModule->pushPacket(packet, referencedGate);
    }

    virtual void pushPacketStart(Packet *packet, bps datarate) {
        referencedModule->pushPacketStart(packet, referencedGate, datarate);
    }

    virtual void pushPacketEnd(Packet *packet) {
        referencedModule->pushPacketEnd(packet, referencedGate);
    }

    virtual void pushPacketProgress(Packet *packet, bps datarate, b position, b extraProcessableLength = b(0)) {
        referencedModule->pushPacketProgress(packet, referencedGate, datarate, position, extraProcessableLength);
    }
};

} // namespace queueing
} // namespace inet

#endif

