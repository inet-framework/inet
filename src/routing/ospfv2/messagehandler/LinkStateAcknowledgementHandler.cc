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

#include "inet/routing/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h"

#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

LinkStateAcknowledgementHandler::LinkStateAcknowledgementHandler(Router *containingRouter) :
    IMessageHandler(containingRouter)
{
}

void LinkStateAcknowledgementHandler::processPacket(OSPFPacket *packet, Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Link State Acknowledgement packet received", intf, neighbor);

    if (neighbor->getState() >= Neighbor::EXCHANGE_STATE) {
        OSPFLinkStateAcknowledgementPacket *lsAckPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket *>(packet);

        int lsaCount = lsAckPacket->getLsaHeadersArraySize();

        EV_DETAIL << "  Processing packet contents:\n";

        for (int i = 0; i < lsaCount; i++) {
            OSPFLSAHeader& lsaHeader = lsAckPacket->getLsaHeaders(i);
            OSPFLSA *lsaOnRetransmissionList;
            LSAKeyType lsaKey;

            EV_DETAIL << "    " << lsaHeader << "\n";

            lsaKey.linkStateID = lsaHeader.getLinkStateID();
            lsaKey.advertisingRouter = lsaHeader.getAdvertisingRouter();

            if ((lsaOnRetransmissionList = neighbor->findOnRetransmissionList(lsaKey)) != NULL) {
                if (operator==(lsaHeader, lsaOnRetransmissionList->getHeader())) {
                    neighbor->removeFromRetransmissionList(lsaKey);
                }
                else {
                    EV_INFO << "Got an Acknowledgement packet for an unsent Update packet.\n";
                }
            }
        }
        if (neighbor->isLinkStateRetransmissionListEmpty()) {
            neighbor->clearUpdateRetransmissionTimer();
        }
    }
}

} // namespace ospf

} // namespace inet

