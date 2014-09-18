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

#include "inet/routing/ospfv2/neighbor/OSPFNeighborStateDown.h"

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighborStateAttempt.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighborStateInit.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

void NeighborStateDown::processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event)
{
    if (event == Neighbor::START) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        int ttl = (neighbor->getInterface()->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        messageHandler->clearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new NeighborStateAttempt, this);
    }
    if (event == Neighbor::HELLO_RECEIVED) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getPollTimer());
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new NeighborStateInit, this);
    }
    if (event == Neighbor::POLL_TIMER) {
        int ttl = (neighbor->getInterface()->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->startTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
    }
}

} // namespace ospf

} // namespace inet

