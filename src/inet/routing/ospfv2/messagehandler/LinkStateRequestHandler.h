//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LINKSTATEREQUESTHANDLER_H
#define __INET_LINKSTATEREQUESTHANDLER_H

#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"

namespace inet {

namespace ospfv2 {

class INET_API LinkStateRequestHandler : public IMessageHandler
{
  public:
    LinkStateRequestHandler(Router *containingRouter);

    void processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *neighbor) override;
};

} // namespace ospfv2

} // namespace inet

#endif

