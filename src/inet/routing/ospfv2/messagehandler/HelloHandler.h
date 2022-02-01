//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_HELLOHANDLER_H
#define __INET_HELLOHANDLER_H

#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"

namespace inet {

namespace ospfv2 {

class INET_API HelloHandler : public IMessageHandler
{
  public:
    HelloHandler(Router *containingRouter);

    void processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *unused = nullptr) override;
};

} // namespace ospfv2

} // namespace inet

#endif

