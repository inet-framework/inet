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

#include "inet/routing/ospfv2/messagehandler/LinkStateRequestHandler.h"

#include "inet/routing/ospfv2/neighbor/OspfNeighbor.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {

namespace ospf {

LinkStateRequestHandler::LinkStateRequestHandler(Router *containingRouter) :
    IMessageHandler(containingRouter)
{
}

void LinkStateRequestHandler::processPacket(Packet *packet, Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Link State Request packet received", intf, neighbor);

    Neighbor::NeighborStateType neighborState = neighbor->getState();

    if ((neighborState == Neighbor::EXCHANGE_STATE) ||
        (neighborState == Neighbor::LOADING_STATE) ||
        (neighborState == Neighbor::FULL_STATE))
    {
        const auto& lsRequestPacket = packet->peekAtFront<OspfLinkStateRequestPacket>();

        unsigned long requestCount = lsRequestPacket->getRequestsArraySize();
        bool error = false;
        std::vector<OspfLsa *> lsas;

        EV_INFO << "  Processing packet contents:\n";

        for (unsigned long i = 0; i < requestCount; i++) {
            const LsaRequest& request = lsRequestPacket->getRequests(i);
            LsaKeyType lsaKey;

            EV_INFO << "    LsaRequest: type=" << request.lsType
                    << ", LSID=" << request.linkStateID
                    << ", advertisingRouter=" << request.advertisingRouter
                    << "\n";

            lsaKey.linkStateID = request.linkStateID;
            lsaKey.advertisingRouter = request.advertisingRouter;

            OspfLsa *lsaInDatabase = router->findLSA(static_cast<LsaType>(request.lsType), lsaKey, intf->getArea()->getAreaID());

            if (lsaInDatabase != nullptr) {
                lsas.push_back(lsaInDatabase);
            }
            else {
                error = true;
                neighbor->processEvent(Neighbor::BAD_LINK_STATE_REQUEST);
                break;
            }
        }

        if (!error) {
            int updatesCount = lsas.size();
            int ttl = (intf->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
            MessageHandler *messageHandler = router->getMessageHandler();

            for (int j = 0; j < updatesCount; j++) {
                Packet *updatePacket = intf->createUpdatePacket(lsas[j]);
                if (updatePacket != nullptr) {
                    if (intf->getType() == Interface::BROADCAST) {
                        if ((intf->getState() == Interface::DESIGNATED_ROUTER_STATE) ||
                            (intf->getState() == Interface::BACKUP_STATE) ||
                            (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
                        {
                            messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                        }
                        else {
                            messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                        }
                    }
                    else {
                        if (intf->getType() == Interface::POINTTOPOINT) {
                            messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                        }
                        else {
                            messageHandler->sendPacket(updatePacket, neighbor->getAddress(), intf->getIfIndex(), ttl);
                        }
                    }
                }
            }
            // These update packets should not be placed on retransmission lists
        }
    }
}

} // namespace ospf

} // namespace inet

