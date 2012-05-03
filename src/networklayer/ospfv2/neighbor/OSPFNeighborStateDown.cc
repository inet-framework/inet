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

#include "MessageHandler.h"
#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "OSPFNeighborStateAttempt.h"
#include "OSPFNeighborStateInit.h"
#include "OSPFRouter.h"


void OSPF::NeighborStateDown::processEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if (event == OSPF::Neighbor::START) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        int ttl = (neighbor->getInterface()->getType() == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        messageHandler->clearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new OSPF::NeighborStateAttempt, this);
    }
    if (event == OSPF::Neighbor::HELLO_RECEIVED) {
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getPollTimer());
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new OSPF::NeighborStateInit, this);
    }
    if (event == OSPF::Neighbor::POLL_TIMER) {
        int ttl = (neighbor->getInterface()->getType() == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        MessageHandler* messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->startTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
    }
}
