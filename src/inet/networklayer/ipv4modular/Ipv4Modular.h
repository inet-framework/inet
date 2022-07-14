//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4MODULAR_H
#define __INET_IPV4MODULAR_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"

namespace inet {

class INET_API Ipv4Modular : public cModule, public INetfilter, public INetworkProtocol
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // INetfilter
    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *daragram) override;
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;
};

} // namespace inet

#endif
