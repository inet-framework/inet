//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateLoading.h"

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateDown.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateExchangeStart.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateFull.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateInit.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateTwoWay.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {
namespace ospfv2 {

void NeighborStateLoading::processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event)
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
    else if (event == Neighbor::ONEWAY_RECEIVED) {
        neighbor->reset();
        changeState(neighbor, new NeighborStateInit, this);
    }
    else if (event == Neighbor::HELLO_RECEIVED) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        messageHandler->startTimer(neighbor->getInactivityTimer(), neighbor->getRouterDeadInterval());
    }
    else if (event == Neighbor::LOADING_DONE) {
        neighbor->clearRequestRetransmissionTimer();
        changeState(neighbor, new NeighborStateFull, this);
    }
    else if (event == Neighbor::IS_ADJACENCY_OK) {
        if (!neighbor->needAdjacency()) {
            neighbor->reset();
            changeState(neighbor, new NeighborStateTwoWay, this);
        }
    }
    else if ((event == Neighbor::SEQUENCE_NUMBER_MISMATCH) || (event == Neighbor::BAD_LINK_STATE_REQUEST)) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->reset();
        neighbor->incrementDDSequenceNumber();
        neighbor->sendDatabaseDescriptionPacket(true);
        messageHandler->startTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
        changeState(neighbor, new NeighborStateExchangeStart, this);
    }
    else if (event == Neighbor::REQUEST_RETRANSMISSION_TIMER) {
        neighbor->sendLinkStateRequestPacket();
        neighbor->startRequestRetransmissionTimer();
    }
    else if (event == Neighbor::UPDATE_RETRANSMISSION_TIMER) {
        neighbor->retransmitUpdatePacket();
        neighbor->startUpdateRetransmissionTimer();
    }
    else if (event == Neighbor::DD_RETRANSMISSION_TIMER) {
        neighbor->deleteLastSentDDPacket();
    }
}

} // namespace ospfv2
} // namespace inet

