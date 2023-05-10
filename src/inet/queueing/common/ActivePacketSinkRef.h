//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSINKREF_H
#define __INET_ACTIVEPACKETSINKREF_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSinkRef : public ModuleRefByGate<IActivePacketSink>
{
  public:
    virtual IPassivePacketSource *getProvider() {
        return referencedModule->getProvider(referencedGate);
    }

    virtual void handleCanPullPacketChanged() {
        referencedModule->handleCanPullPacketChanged(referencedGate);
    }

    virtual void handlePullPacketProcessed(Packet *packet, bool successful) {
        referencedModule->handlePullPacketProcessed(packet, referencedGate, successful);
    }
};

} // namespace queueing
} // namespace inet

#endif

