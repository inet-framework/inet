//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_INetfilterCompatibleIpv4HookManagerBase_H
#define __INET_INetfilterCompatibleIpv4HookManagerBase_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/ipv4modular/IIpv4HookManager.h"

namespace inet {

class INET_API INetfilterCompatibleIpv4HookManagerBase : public INetfilter, public IIpv4HookManager
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
    typedef std::map<IHook*, IHookHandlers> IHookInfo;
    IHookInfo hookInfo;

  public:
    static Ipv4Hook::NetfilterResult mapResult(INetfilter::IHook::Result r);

    // INetfilter compatibility:
    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *datagram) override;
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;
};

} // namespace inet

#endif
