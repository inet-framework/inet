//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCEREF_H
#define __INET_PASSIVEPACKETSOURCEREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PassivePacketSourceRef : public ModuleRefByGate<IPassivePacketSource>
{
  public:
    virtual ~PassivePacketSourceRef() {}

    virtual bool canPullSomePacket() const {
        return referencedModule->canPullSomePacket(referencedGate);
    }

    virtual Packet *canPullPacket() const {
        return referencedModule->canPullPacket(referencedGate);
    }

    virtual Packet *pullPacket() {
        return referencedModule->pullPacket(referencedGate);
    }

    virtual Packet *pullPacketStart(bps datarate) {
        return referencedModule->pullPacketStart(referencedGate, datarate);
    }

    virtual Packet *pullPacketEnd() {
        return referencedModule->pullPacketEnd(referencedGate);
    }

    virtual Packet *pullPacketProgress(bps datarate, b position, b extraProcessableLength) {
        return referencedModule->pullPacketProgress(referencedGate, datarate, position, extraProcessableLength);
    }
};

} // namespace queueing
} // namespace inet

#endif

