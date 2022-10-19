//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4MODULAR_H
#define __INET_IPV4MODULAR_H

#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/contract/netfilter/INetfilterCompatibleNetfilterHookManagerBase.h"

namespace inet {

class INET_API Ipv4Modular : public cModule, public NetfilterHook::INetfilterCompatibleNetfilterHookManagerBase, public INetworkProtocol
{
  protected:
    opp_component_ptr<INetfilterHookManager> hookManager;

  protected:
    void chkHookManager();

  public:
    // cModule:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // INetfilterHookManager:
    virtual void registerNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(NetfilterHook::NetfilterType type, int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void reinjectDatagram(Packet *datagram, NetfilterHook::NetfilterResult action) override;
};

} // namespace inet

#endif
