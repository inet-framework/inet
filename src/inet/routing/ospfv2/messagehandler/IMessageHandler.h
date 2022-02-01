//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IMESSAGEHANDLER_H
#define __INET_IMESSAGEHANDLER_H

#include "inet/common/packet/Packet.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"

namespace inet {

namespace ospfv2 {

class Router;
class Ospfv2Interface;
class Neighbor;

class INET_API IMessageHandler
{
  protected:
    Router *router;

  public:
    IMessageHandler(Router *containingRouter) { router = containingRouter; }
    virtual ~IMessageHandler() {}

    virtual void processPacket(Packet *, Ospfv2Interface *intf, Neighbor *neighbor) = 0;
};

} // namespace ospfv2

} // namespace inet

#endif

