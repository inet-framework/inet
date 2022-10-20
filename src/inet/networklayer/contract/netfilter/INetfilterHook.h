//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_INETFILTERHOOK_H
#define __INET_INETFILTERHOOK_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/netfilter/INetfilterHookManager.h"

namespace inet {
namespace NetfilterHook {

class INET_API INetfilterHook
{
    virtual void registerNetfilterHandler(int priority, NetfilterHandler *handler) = 0;
    virtual void unregisterNetfilterHandler(int priority, NetfilterHandler *handler) = 0;
    virtual void reinjectDatagram(Packet *datagram, NetfilterResult action) = 0;
};

} // namespace NetfilterHook
} // namespace inet

#endif
