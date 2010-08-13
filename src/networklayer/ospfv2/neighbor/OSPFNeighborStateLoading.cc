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

#include "OSPFNeighborStateLoading.h"
#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateInit.h"
#include "OSPFNeighborStateFull.h"
#include "OSPFNeighborStateTwoWay.h"
#include "OSPFNeighborStateExchangeStart.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateLoading::processEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if ((event == OSPF::Neighbor::KillNeighbor) || (event == OSPF::Neighbor::LinkDown)) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->Reset();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::InactivityTimer) {
        neighbor->Reset();
        if (neighbor->getInterface()->getType() == OSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->StartTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
        }
        changeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::OneWayReceived) {
        neighbor->Reset();
        changeState(neighbor, new OSPF::NeighborStateInit, this);
    }
    if (event == OSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        messageHandler->StartTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
    }
    if (event == OSPF::Neighbor::LoadingDone) {
        neighbor->clearRequestRetransmissionTimer();
        changeState(neighbor, new OSPF::NeighborStateFull, this);
    }
    if (event == OSPF::Neighbor::IsAdjacencyOK) {
        if (!neighbor->NeedAdjacency()) {
            neighbor->Reset();
            changeState(neighbor, new OSPF::NeighborStateTwoWay, this);
        }
    }
    if ((event == OSPF::Neighbor::SequenceNumberMismatch) || (event == OSPF::Neighbor::BadLinkStateRequest)) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->Reset();
        neighbor->incrementDDSequenceNumber();
        neighbor->sendDatabaseDescriptionPacket(true);
        messageHandler->StartTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
        changeState(neighbor, new OSPF::NeighborStateExchangeStart, this);
    }
    if (event == OSPF::Neighbor::RequestRetransmissionTimer) {
        neighbor->sendLinkStateRequestPacket();
        neighbor->StartRequestRetransmissionTimer();
    }
    if (event == OSPF::Neighbor::UpdateRetransmissionTimer) {
        neighbor->retransmitUpdatePacket();
        neighbor->StartUpdateRetransmissionTimer();
    }
    if (event == OSPF::Neighbor::DDRetransmissionTimer) {
        neighbor->DeleteLastSentDDPacket();
    }
}
