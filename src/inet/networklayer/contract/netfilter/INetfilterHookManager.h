//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_INETFILTERHOOKMANAGER_H
#define __INET_INETFILTERHOOKMANAGER_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/netfilter/NetfilterHookDefs_m.h"

namespace inet {
namespace NetfilterHook {

class INET_API INetfilterHookManager
{
  public:
    virtual void registerNetfilterHandler(NetfilterType type, int priority, NetfilterHandler *handler) = 0;
    virtual void unregisterNetfilterHandler(NetfilterType type, int priority, NetfilterHandler *handler) = 0;
    virtual void reinjectDatagram(Packet *datagram, NetfilterResult action) = 0;
};

} // namespace NetfilterHook
} // namespace inet

#endif
