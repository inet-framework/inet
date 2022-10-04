//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4MODULAR_H
#define __INET_IPV4MODULAR_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/ipv4modular/IIpv4HookManager.h"

namespace inet {

class INET_API Ipv4Modular : public cModule, public INetfilter, public IIpv4HookManager, public INetworkProtocol
{
  protected:
    class IHookHandlers {
      public:
        int priority = -1;
        Ipv4Hook::NetfilterHandler *prerouting = nullptr;
        Ipv4Hook::NetfilterHandler *localIn = nullptr;
        Ipv4Hook::NetfilterHandler *forward = nullptr;
        Ipv4Hook::NetfilterHandler *postrouting = nullptr;
        Ipv4Hook::NetfilterHandler *localOut = nullptr;
    };
    opp_component_ptr<IIpv4HookManager> hookManager;
    typedef std::map<IHook*, IHookHandlers> IHookInfo;
    IHookInfo hookInfo;

  protected:
    void chkHookManager();
    // cModule:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // IIpv4HookManager:
    virtual void registerNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) override;
    virtual void reinjectQueuedDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action) override;

    // INetfilter compatibility:
    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *datagram) override;
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;
};

} // namespace inet

#endif
