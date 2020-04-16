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

#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateDown.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateLoopback.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateNotDesignatedRouter.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStatePointToPoint.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateWaiting.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

void InterfaceStateDown::processEvent(Ospfv2Interface *intf, Ospfv2Interface::Ospfv2InterfaceEventType event)
{
    if (event == Ospfv2Interface::INTERFACE_UP) {
        MessageHandler *messageHandler = intf->getArea()->getRouter()->getMessageHandler();
        messageHandler->startTimer(intf->getHelloTimer(), RNGCONTEXT truncnormal(0.1, 0.01));    // add some deviation to avoid startup collisions
        messageHandler->startTimer(intf->getAcknowledgementTimer(), intf->getAcknowledgementDelay());
        switch (intf->getType()) {
            case Ospfv2Interface::POINTTOPOINT:
            case Ospfv2Interface::POINTTOMULTIPOINT:
            case Ospfv2Interface::VIRTUAL:
                changeState(intf, new InterfaceStatePointToPoint, this);
                break;

            case Ospfv2Interface::NBMA:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new InterfaceStateNotDesignatedRouter, this);
                }
                else {
                    changeState(intf, new InterfaceStateWaiting, this);
                    messageHandler->startTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());

                    long neighborCount = intf->getNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        Neighbor *neighbor = intf->getNeighbor(i);
                        if (neighbor->getPriority() > 0) {
                            neighbor->processEvent(Neighbor::START);
                        }
                    }
                }
                break;

            case Ospfv2Interface::BROADCAST:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new InterfaceStateNotDesignatedRouter, this);
                }
                else {
                    changeState(intf, new InterfaceStateWaiting, this);
                    messageHandler->startTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());
                }
                break;

            default:
                break;
        }
    }
    else if (event == Ospfv2Interface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new InterfaceStateLoopback, this);
    }
}

} // namespace ospfv2

} // namespace inet

