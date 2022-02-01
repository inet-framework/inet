//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h"

#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {
namespace ospfv2 {

LinkStateAcknowledgementHandler::LinkStateAcknowledgementHandler(Router *containingRouter) :
    IMessageHandler(containingRouter)
{
}

void LinkStateAcknowledgementHandler::processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Link State Acknowledgement packet received", intf, neighbor);

    if (neighbor->getState() >= Neighbor::EXCHANGE_STATE) {
        const auto& lsAckPacket = packet->peekAtFront<Ospfv2LinkStateAcknowledgementPacket>();

        int lsaCount = lsAckPacket->getLsaHeadersArraySize();

        EV_DETAIL << "  Processing packet contents:\n";

        for (int i = 0; i < lsaCount; i++) {
            const Ospfv2LsaHeader& lsaHeader = lsAckPacket->getLsaHeaders(i);
            Ospfv2Lsa *lsaOnRetransmissionList;
            LsaKeyType lsaKey;

            EV_DETAIL << "    " << lsaHeader << "\n";

            lsaKey.linkStateID = lsaHeader.getLinkStateID();
            lsaKey.advertisingRouter = lsaHeader.getAdvertisingRouter();

            if ((lsaOnRetransmissionList = neighbor->findOnRetransmissionList(lsaKey)) != nullptr) {
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

} // namespace ospfv2
} // namespace inet

