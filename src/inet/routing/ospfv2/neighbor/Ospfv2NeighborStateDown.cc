//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateDown.h"

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateAttempt.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateInit.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

void NeighborStateDown::processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event)
{
    if (event == Neighbor::START) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        int ttl = (neighbor->getInterface()->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        messageHandler->clearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new NeighborStateAttempt, this);
    }
    else if (event == Neighbor::HELLO_RECEIVED) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getPollTimer());
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new NeighborStateInit, this);
    }
    else if (event == Neighbor::POLL_TIMER) {
        int ttl = (neighbor->getInterface()->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->sendHelloPacket(neighbor->getAddress(), ttl);
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->startTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
    }
}

} // namespace ospfv2

} // namespace inet

