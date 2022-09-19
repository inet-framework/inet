//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4DECAPSULATE_H
#define __INET_IPV4DECAPSULATE_H

#include "inet/networklayer/ipv4modular/IIpv4HookManager.h"
#include "inet/networklayer/ipv4modular/Ipv4NetfilterHook.h"

namespace inet {

class INET_API Ipv4HookManager : public cSimpleModule, public IIpv4HookManager
{
  protected:
    opp_component_ptr<Ipv4NetfilterHook> hook[Ipv4Hook::NetfilterType::__NUM_HOOK_TYPES];

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void registerNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) override;
    virtual void reinjectQueuedDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action) override;
};

} // namespace inet

#endif
