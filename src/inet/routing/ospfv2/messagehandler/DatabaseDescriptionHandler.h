//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DATABASEDESCRIPTIONHANDLER_H
#define __INET_DATABASEDESCRIPTIONHANDLER_H

#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"

namespace inet {

namespace ospfv2 {

class INET_API DatabaseDescriptionHandler : public IMessageHandler
{
  private:
    bool processDDPacket(const Ospfv2DatabaseDescriptionPacket *ddPacket, Ospfv2Interface *intf, Neighbor *neighbor, bool inExchangeStart);

  public:
    DatabaseDescriptionHandler(Router *containingRouter);

    void processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *neighbor) override;
};

} // namespace ospfv2

} // namespace inet

#endif

