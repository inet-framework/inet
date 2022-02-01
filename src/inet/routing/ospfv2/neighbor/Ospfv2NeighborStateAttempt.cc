//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateAttempt.h"

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateDown.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateInit.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

void NeighborStateAttempt::processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event)
{
    if ((event == Neighbor::KILL_NEIGHBOR) || (event == Neighbor::LINK_DOWN)) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->reset();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new NeighborStateDown, this);
    }
    else if (event == Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == Ospfv2Interface::NBMA) {
            MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->startTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
        }
        changeState(neighbor, new NeighborStateDown, this);
    }
    else if (event == Neighbor::HELLO_RECEIVED) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
        changeState(neighbor, new NeighborStateInit, this);
    }
}

} // namespace ospfv2

} // namespace inet

