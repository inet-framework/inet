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


#include "OSPFInterfaceStateDown.h"

#include "MessageHandler.h"
#include "OSPFArea.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFInterfaceStateNotDesignatedRouter.h"
#include "OSPFInterfaceStatePointToPoint.h"
#include "OSPFInterfaceStateWaiting.h"
#include "OSPFRouter.h"


void OSPF::InterfaceStateDown::processEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::INTERFACE_UP) {
        OSPF::MessageHandler* messageHandler = intf->getArea()->getRouter()->getMessageHandler();
        messageHandler->startTimer(intf->getHelloTimer(), truncnormal(0.1, 0.01)); // add some deviation to avoid startup collisions
        messageHandler->startTimer(intf->getAcknowledgementTimer(), intf->getAcknowledgementDelay());
        switch (intf->getType()) {
            case OSPF::Interface::POINTTOPOINT:
            case OSPF::Interface::POINTTOMULTIPOINT:
            case OSPF::Interface::VIRTUAL:
                changeState(intf, new OSPF::InterfaceStatePointToPoint, this);
                break;

            case OSPF::Interface::NBMA:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    changeState(intf, new OSPF::InterfaceStateWaiting, this);
                    messageHandler->startTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());

                    long neighborCount = intf->getNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        OSPF::Neighbor* neighbor = intf->getNeighbor(i);
                        if (neighbor->getPriority() > 0) {
                            neighbor->processEvent(OSPF::Neighbor::START);
                        }
                    }
                }
                break;

            case OSPF::Interface::BROADCAST:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    changeState(intf, new OSPF::InterfaceStateWaiting, this);
                    messageHandler->startTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());
                }
                break;

            default:
                break;
        }
    }
    if (event == OSPF::Interface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new OSPF::InterfaceStateLoopback, this);
    }
}

