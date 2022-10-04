//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIPV4HOOKMANAGER_H
#define __INET_IIPV4HOOKMANAGER_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv4modular/IIpv4HookManager_m.h"

namespace inet {

class INET_API IIpv4HookManager
{
  public:
    virtual void registerNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) = 0;
    virtual void unregisterNetfilterHandler(Ipv4Hook::NetfilterType type, int priority, Ipv4Hook::NetfilterHandler *handler) = 0;
    virtual void reinjectDatagram(Packet *datagram, Ipv4Hook::NetfilterResult action) = 0;
};

} // namespace inet

#endif
