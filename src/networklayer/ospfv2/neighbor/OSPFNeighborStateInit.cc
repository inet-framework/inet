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

#include "OSPFNeighborStateInit.h"
#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateExchangeStart.h"
#include "OSPFNeighborStateTwoWay.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateInit::ProcessEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if ((event == OSPF::Neighbor::KillNeighbor) || (event == OSPF::Neighbor::LinkDown)) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->Reset();
        messageHandler->ClearTimer(neighbor->getInactivityTimer());
        ChangeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::InactivityTimer) {
        neighbor->Reset();
        if (neighbor->getInterface()->getType() == OSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->StartTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
        }
        ChangeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->ClearTimer(neighbor->getInactivityTimer());
        messageHandler->StartTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
    }
    if (event == OSPF::Neighbor::TwoWayReceived) {
        if (neighbor->NeedAdjacency()) {
            MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            if (!(neighbor->IsFirstAdjacencyInited())) {
                neighbor->InitFirstAdjacency();
            } else {
                neighbor->IncrementDDSequenceNumber();
            }
            neighbor->SendDatabaseDescriptionPacket(true);
            messageHandler->StartTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
            ChangeState(neighbor, new OSPF::NeighborStateExchangeStart, this);
        } else {
            ChangeState(neighbor, new OSPF::NeighborStateTwoWay, this);
        }
    }
}
