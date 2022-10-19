//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4HOOKMANAGER_H
#define __INET_IPV4HOOKMANAGER_H

#include "inet/networklayer/contract/netfilter/INetfilterHookManager.h"
#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4NetfilterHook.h"

namespace inet {

class INET_API Ipv4HookManager : public cSimpleModule, public NetfilterHook::INetfilterHookManager
{
  protected:
    opp_component_ptr<Ipv4NetfilterHook> hook[NetfilterHook::NetfilterType::__NUM_HOOK_TYPES];

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action) override;
};

} // namespace inet

#endif
