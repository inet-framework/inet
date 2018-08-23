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

#include "inet/routing/ospfv2/neighbor/OspfNeighborStateFull.h"

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/interface/OspfInterface.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborStateDown.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborStateExchangeStart.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborStateInit.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborStateTwoWay.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {

namespace ospf {

void NeighborStateFull::processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event)
{
    if ((event == Neighbor::KILL_NEIGHBOR) || (event == Neighbor::LINK_DOWN)) {
        MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
        neighbor->reset();
        messageHandler->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new NeighborStateDown, this);
    }
    else if (event == Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == OspfInterface::NBMA) {
            MessageHandler *messageHandler = neighbor->getInterface()->getArea()->getRouter()->getMessageHandler();
            messageHandler->startTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());
        }
        changeState(neighbor, new NeighborStateDown, this);

        if (neighbor->getInterface()->getState() == OspfInterface::BACKUP_STATE &&
                neighbor->getInterface()->getDesignatedRouter().routerID == neighbor->getNeighborID())
            neighbor->getInterface()->processEvent(OspfInterface::NEIGHBOR_CHANGE);
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
    else if (event == Neighbor::UPDATE_RETRANSMISSION_TIMER) {
        neighbor->retransmitUpdatePacket();
        neighbor->startUpdateRetransmissionTimer();
    }
    else if (event == Neighbor::DD_RETRANSMISSION_TIMER) {
        neighbor->deleteLastSentDDPacket();
    }
}

} // namespace ospf

} // namespace inet

