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

#include "OSPFNeighborStateExchange.h"
#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateInit.h"
#include "OSPFNeighborStateTwoWay.h"
#include "OSPFNeighborStateExchangeStart.h"
#include "OSPFNeighborStateFull.h"
#include "OSPFNeighborStateLoading.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateExchange::processEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if ((event == OSPF::Neighbor::KILL_NEIGHBOR) || (event == OSPF::Neighbor::LINK_DOWN)) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->Reset();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::INACTIVITY_TIMER) {
        neighbor->Reset();
        if (neighbor->getInterface()->getType() == OSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->StartTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
        }
        changeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::ONEWAY_RECEIVED) {
        neighbor->Reset();
        changeState(neighbor, new OSPF::NeighborStateInit, this);
    }
    if (event == OSPF::Neighbor::HELLO_RECEIVED) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        messageHandler->StartTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
    }
    if (event == OSPF::Neighbor::IS_ADJACENCY_OK) {
        if (!neighbor->NeedAdjacency()) {
            neighbor->Reset();
            changeState(neighbor, new OSPF::NeighborStateTwoWay, this);
        }
    }
    if ((event == OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH) || (event == OSPF::Neighbor::BAD_LINK_STATE_REQUEST)) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->Reset();
        neighbor->incrementDDSequenceNumber();
        neighbor->sendDatabaseDescriptionPacket(true);
        messageHandler->StartTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
        changeState(neighbor, new OSPF::NeighborStateExchangeStart, this);
    }
    if (event == OSPF::Neighbor::EXCHANGE_DONE) {
        if (neighbor->IsLinkStateRequestListEmpty()) {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->StartTimer(neighbor->getDDRetransmissionTimer(), neighbor->getRouterDeadInterval());
            neighbor->clearRequestRetransmissionTimer();
            changeState(neighbor, new OSPF::NeighborStateFull, this);
        } else {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->StartTimer(neighbor->getDDRetransmissionTimer(), neighbor->getRouterDeadInterval());
            changeState(neighbor, new OSPF::NeighborStateLoading, this);
        }
    }
    if (event == OSPF::Neighbor::UPDATE_RETRANSMISSION_TIMER) {
        neighbor->retransmitUpdatePacket();
        neighbor->StartUpdateRetransmissionTimer();
    }
    if (event == OSPF::Neighbor::REQUEST_RETRANSMISSION_TIMER) {
        neighbor->sendLinkStateRequestPacket();
        neighbor->StartRequestRetransmissionTimer();
    }
}
