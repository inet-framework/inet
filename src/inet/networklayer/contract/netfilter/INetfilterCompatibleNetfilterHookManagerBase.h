//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_INetfilterCompatibleIpv4HookManagerBase_H
#define __INET_INetfilterCompatibleIpv4HookManagerBase_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/contract/netfilter/INetfilterHookManager.h"

namespace inet {
namespace NetfilterHook {

class INET_API INetfilterCompatibleNetfilterHookManagerBase : public INetfilter, public INetfilterHookManager
{
  protected:
    class IHookHandlers {
      public:
        int priority = -1;
        NetfilterHandler *prerouting = nullptr;
        NetfilterHandler *localIn = nullptr;
        NetfilterHandler *forward = nullptr;
        NetfilterHandler *postrouting = nullptr;
        NetfilterHandler *localOut = nullptr;
    };
    typedef std::map<IHook*, IHookHandlers> IHookInfo;
    IHookInfo hookInfo;

  public:
    static NetfilterResult mapResult(INetfilter::IHook::Result r);

    // INetfilter compatibility:
    virtual void registerHook(int priority, IHook *hook) override;
    virtual void unregisterHook(IHook *hook) override;
    virtual void dropQueuedDatagram(const Packet *datagram) override;
    virtual void reinjectQueuedDatagram(const Packet *datagram) override;
};

} // namespace NetfilterHook
} // namespace inet

#endif
