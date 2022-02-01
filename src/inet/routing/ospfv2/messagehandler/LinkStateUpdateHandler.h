//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LINKSTATEUPDATEHANDLER_H
#define __INET_LINKSTATEUPDATEHANDLER_H

#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {

namespace ospfv2 {

class INET_API LinkStateUpdateHandler : public IMessageHandler
{
  private:
    struct AcknowledgementFlags {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

  private:
    bool validateLSChecksum(const Ospfv2Lsa *lsa) { return true; } // not implemented
    void acknowledgeLSA(const Ospfv2LsaHeader& lsaHeader, Ospfv2Interface *intf, AcknowledgementFlags acknowledgementFlags, RouterId lsaSource);

  public:
    LinkStateUpdateHandler(Router *containingRouter);

    void processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *neighbor) override;
};

} // namespace ospfv2

} // namespace inet

#endif

