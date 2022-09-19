//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4NETFILTERHOOK_H
#define __INET_IPV4NETFILTERHOOK_H

#include "inet/networklayer/ipv4modular/IIpv4HookManager.h"
#include "inet/networklayer/ipv4modular/IIpv4NetfilterHook.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4NetfilterHook : public queueing::PacketPusherBase, public IIpv4NetfilterHook
{
  protected:
    class Item {
      public:
        int priority;
        Ipv4Hook::NetfilterHandler *handler;
        Item(int priority, Ipv4Hook::NetfilterHandler *handler) : priority(priority), handler(handler) {}
    };
    typedef std::vector<Item> Items;
    Items handlers;

  protected:
    Ipv4NetfilterHook::Items::iterator findHandler(int priority, const Ipv4Hook::NetfilterHandler *handler);
    Ipv4Hook::NetfilterResult iterateHandlers(Packet *packet, Items::iterator it);
    Ipv4Hook::NetfilterResult iterateHandlers(Packet *packet) { return iterateHandlers(packet, handlers.begin()); }

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void registerNetfilterHandler(int priority, Ipv4Hook::NetfilterHandler *handler) override;
    virtual void unregisterNetfilterHandler(int priority, Ipv4Hook::NetfilterHandler *handler) override;

    virtual void reinjectQueuedDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action) override;
};

} // namespace inet

#endif
