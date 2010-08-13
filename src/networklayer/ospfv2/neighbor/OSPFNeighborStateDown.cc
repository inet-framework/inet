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

#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateAttempt.h"
#include "OSPFNeighborStateInit.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateDown::ProcessEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if (event == OSPF::Neighbor::Start) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        int             ttl            = (neighbor->getInterface()->GetType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

        messageHandler->ClearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->SendHelloPacket(neighbor->getAddress(), ttl);
        messageHandler->StartTimer(neighbor->getInactivityTimer(), neighbor->GetRouterDeadInterval());
        ChangeState(neighbor, new OSPF::NeighborStateAttempt, this);
    }
    if (event == OSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->ClearTimer(neighbor->getPollTimer());
        messageHandler->StartTimer(neighbor->getInactivityTimer(), neighbor->GetRouterDeadInterval());
        ChangeState(neighbor, new OSPF::NeighborStateInit, this);
    }
    if (event == OSPF::Neighbor::PollTimer) {
        int ttl = (neighbor->getInterface()->GetType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->SendHelloPacket(neighbor->getAddress(), ttl);
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->StartTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
    }
}
