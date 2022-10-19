//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4NETFILTERHOOK_H
#define __INET_IPV4NETFILTERHOOK_H

#include "inet/networklayer/contract/netfilter/INetfilterHook.h"
#include "inet/networklayer/contract/netfilter/INetfilterHookManager.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4NetfilterHook : public queueing::PacketPusherBase, public NetfilterHook::INetfilterHook
{
  protected:
    class Item {
      public:
        int priority;
        NetfilterHook::NetfilterHandler *handler;
        Item(int priority, NetfilterHook::NetfilterHandler *handler) : priority(priority), handler(handler) {}
    };
    typedef std::vector<Item> Items;
    Items handlers;

  protected:
    Ipv4NetfilterHook::Items::iterator findHandler(int priority, const NetfilterHook::NetfilterHandler *handler);
    NetfilterHook::NetfilterResult iterateHandlers(Packet *packet, Items::iterator it);
    NetfilterHook::NetfilterResult iterateHandlers(Packet *packet) { return iterateHandlers(packet, handlers.begin()); }

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void registerNetfilterHandler(int priority, NetfilterHook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(int priority, NetfilterHook::NetfilterHandler *handler) override;

    virtual void reinjectQueuedDatagram(Packet *datagram, NetfilterHook::NetfilterResult action) override;
};

} // namespace inet

#endif
