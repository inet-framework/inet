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

#include "LinkStateRequestHandler.h"
#include "OSPFNeighbor.h"
#include "OSPFRouter.h"
#include <vector>

OSPF::LinkStateRequestHandler::LinkStateRequestHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

void OSPF::LinkStateRequestHandler::processPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->getMessageHandler()->printEvent("Link State Request packet received", intf, neighbor);

    OSPF::Neighbor::NeighborStateType neighborState = neighbor->getState();

    if ((neighborState == OSPF::Neighbor::ExchangeState) ||
        (neighborState == OSPF::Neighbor::LoadingState) ||
        (neighborState == OSPF::Neighbor::FullState))
    {
        OSPFLinkStateRequestPacket* lsRequestPacket = check_and_cast<OSPFLinkStateRequestPacket*> (packet);

        unsigned long         requestCount = lsRequestPacket->getRequestsArraySize();
        bool                  error        = false;
        std::vector<OSPFLSA*> lsas;

        EV << "  Processing packet contents:\n";

        for (unsigned long i = 0; i < requestCount; i++) {
            LSARequest&      request = lsRequestPacket->getRequests(i);
            OSPF::LSAKeyType lsaKey;
            char             addressString[16];

            EV << "    LSARequest: type="
               << request.lsType
               << ", LSID="
               << AddressStringFromULong(addressString, sizeof(addressString), request.linkStateID)
               << ", advertisingRouter="
               << AddressStringFromULong(addressString, sizeof(addressString), request.advertisingRouter.getInt())
               << "\n";

            lsaKey.linkStateID = request.linkStateID;
            lsaKey.advertisingRouter = request.advertisingRouter.getInt();

            OSPFLSA* lsaInDatabase = router->findLSA(static_cast<LSAType> (request.lsType), lsaKey, intf->getArea()->getAreaID());

            if (lsaInDatabase != NULL) {
                lsas.push_back(lsaInDatabase);
            } else {
                error = true;
                neighbor->processEvent(OSPF::Neighbor::BadLinkStateRequest);
                break;
            }
        }

        if (!error) {
            int                   updatesCount   = lsas.size();
            int                   ttl            = (intf->getType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
            OSPF::MessageHandler* messageHandler = router->getMessageHandler();

            for (int j = 0; j < updatesCount; j++) {
                OSPFLinkStateUpdatePacket* updatePacket = intf->createUpdatePacket(lsas[j]);
                if (updatePacket != NULL) {
                    if (intf->getType() == OSPF::Interface::Broadcast) {
                        if ((intf->getState() == OSPF::Interface::DesignatedRouterState) ||
                            (intf->getState() == OSPF::Interface::BackupState) ||
                            (intf->getDesignatedRouter() == OSPF::NullDesignatedRouterID))
                        {
                            messageHandler->sendPacket(updatePacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
                        } else {
                            messageHandler->sendPacket(updatePacket, OSPF::AllDRouters, intf->getIfIndex(), ttl);
                        }
                    } else {
                        if (intf->getType() == OSPF::Interface::PointToPoint) {
                            messageHandler->sendPacket(updatePacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
                        } else {
                            messageHandler->sendPacket(updatePacket, neighbor->getAddress(), intf->getIfIndex(), ttl);
                        }
                    }

                }
            }
            // These update packets should not be placed on retransmission lists
        }
    }
}
