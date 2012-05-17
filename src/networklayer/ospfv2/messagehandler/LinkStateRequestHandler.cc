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


#include <vector>

#include "LinkStateRequestHandler.h"

#include "OSPFNeighbor.h"
#include "OSPFRouter.h"


OSPF::LinkStateRequestHandler::LinkStateRequestHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

void OSPF::LinkStateRequestHandler::processPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->getMessageHandler()->printEvent("Link State Request packet received", intf, neighbor);

    OSPF::Neighbor::NeighborStateType neighborState = neighbor->getState();

    if ((neighborState == OSPF::Neighbor::EXCHANGE_STATE) ||
        (neighborState == OSPF::Neighbor::LOADING_STATE) ||
        (neighborState == OSPF::Neighbor::FULL_STATE))
    {
        OSPFLinkStateRequestPacket* lsRequestPacket = check_and_cast<OSPFLinkStateRequestPacket*> (packet);

        unsigned long requestCount = lsRequestPacket->getRequestsArraySize();
        bool error = false;
        std::vector<OSPFLSA*> lsas;

        EV << "  Processing packet contents:\n";

        for (unsigned long i = 0; i < requestCount; i++) {
            LSARequest& request = lsRequestPacket->getRequests(i);
            OSPF::LSAKeyType lsaKey;

            EV << "    LSARequest: type=" << request.lsType
               << ", LSID=" << request.linkStateID
               << ", advertisingRouter=" << request.advertisingRouter
               << "\n";

            lsaKey.linkStateID = request.linkStateID;
            lsaKey.advertisingRouter = request.advertisingRouter;

            OSPFLSA* lsaInDatabase = router->findLSA(static_cast<LSAType> (request.lsType), lsaKey, intf->getArea()->getAreaID());

            if (lsaInDatabase != NULL) {
                lsas.push_back(lsaInDatabase);
            } else {
                error = true;
                neighbor->processEvent(OSPF::Neighbor::BAD_LINK_STATE_REQUEST);
                break;
            }
        }

        if (!error) {
            int updatesCount = lsas.size();
            int ttl = (intf->getType() == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
            OSPF::MessageHandler* messageHandler = router->getMessageHandler();

            for (int j = 0; j < updatesCount; j++) {
                OSPFLinkStateUpdatePacket* updatePacket = intf->createUpdatePacket(lsas[j]);
                if (updatePacket != NULL) {
                    if (intf->getType() == OSPF::Interface::BROADCAST) {
                        if ((intf->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                            (intf->getState() == OSPF::Interface::BACKUP_STATE) ||
                            (intf->getDesignatedRouter() == OSPF::NULL_DESIGNATEDROUTERID))
                        {
                            messageHandler->sendPacket(updatePacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                        } else {
                            messageHandler->sendPacket(updatePacket, IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                        }
                    } else {
                        if (intf->getType() == OSPF::Interface::POINTTOPOINT) {
                            messageHandler->sendPacket(updatePacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
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
