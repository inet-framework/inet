//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_LINKSTATEUPDATEHANDLER_H
#define __INET_LINKSTATEUPDATEHANDLER_H

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"

namespace inet {

namespace ospf {

class INET_API LinkStateUpdateHandler : public IMessageHandler
{
  private:
    struct AcknowledgementFlags
    {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

  private:
    bool validateLSChecksum(const OspfLsa *lsa) { return true; }    // not implemented
    void acknowledgeLSA(const OspfLsaHeader& lsaHeader, OspfInterface *intf, AcknowledgementFlags acknowledgementFlags, RouterId lsaSource);

  public:
    LinkStateUpdateHandler(Router *containingRouter);

    void processPacket(Packet *packet, OspfInterface *intf, Neighbor *neighbor) override;
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_LINKSTATEUPDATEHANDLER_H

