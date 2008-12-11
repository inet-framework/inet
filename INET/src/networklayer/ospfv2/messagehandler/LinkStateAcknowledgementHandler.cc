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

#include "LinkStateAcknowledgementHandler.h"
#include "OSPFRouter.h"

OSPF::LinkStateAcknowledgementHandler::LinkStateAcknowledgementHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

void OSPF::LinkStateAcknowledgementHandler::ProcessPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->GetMessageHandler()->PrintEvent("Link State Acknowledgement packet received", intf, neighbor);

    if (neighbor->GetState() >= OSPF::Neighbor::ExchangeState) {
        OSPFLinkStateAcknowledgementPacket* lsAckPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket*> (packet);

        int  lsaCount = lsAckPacket->getLsaHeadersArraySize();

        EV << "  Processing packet contents:\n";

        for (int i  = 0; i < lsaCount; i++) {
            OSPFLSAHeader&   lsaHeader = lsAckPacket->getLsaHeaders(i);
            OSPFLSA*         lsaOnRetransmissionList;
            OSPF::LSAKeyType lsaKey;

            EV << "    ";
            PrintLSAHeader(lsaHeader, ev.getOStream());
            EV << "\n";

            lsaKey.linkStateID = lsaHeader.getLinkStateID();
            lsaKey.advertisingRouter = lsaHeader.getAdvertisingRouter().getInt();

            if ((lsaOnRetransmissionList = neighbor->FindOnRetransmissionList(lsaKey)) != NULL) {
                if (operator== (lsaHeader, lsaOnRetransmissionList->getHeader())) {
                    neighbor->RemoveFromRetransmissionList(lsaKey);
                } else {
                    EV << "Got an Acknowledgement packet for an unsent Update packet.\n";
                }
            }
        }
        if (neighbor->IsLinkStateRetransmissionListEmpty()) {
            neighbor->ClearUpdateRetransmissionTimer();
        }
    }
}
