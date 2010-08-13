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
#include "OSPFRouter.h"
#include "OSPFInterfaceStatePointToPoint.h"
#include "OSPFInterfaceStateNotDesignatedRouter.h"
#include "OSPFInterfaceStateWaiting.h"
#include "OSPFInterfaceStateLoopback.h"

void OSPF::InterfaceStateDown::processEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::InterfaceUp) {
        OSPF::MessageHandler* messageHandler = intf->getArea()->getRouter()->getMessageHandler();
        messageHandler->StartTimer(intf->getHelloTimer(), truncnormal(0.1, 0.01)); // add some deviation to avoid startup collisions
        messageHandler->StartTimer(intf->getAcknowledgementTimer(), intf->getAcknowledgementDelay());
        switch (intf->getType()) {
            case OSPF::Interface::PointToPoint:
            case OSPF::Interface::PointToMultiPoint:
            case OSPF::Interface::Virtual:
                changeState(intf, new OSPF::InterfaceStatePointToPoint, this);
                break;

            case OSPF::Interface::NBMA:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    changeState(intf, new OSPF::InterfaceStateWaiting, this);
                    messageHandler->StartTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());

                    long neighborCount = intf->getNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        OSPF::Neighbor* neighbor = intf->getNeighbor(i);
                        if (neighbor->getPriority() > 0) {
                            neighbor->processEvent(OSPF::Neighbor::Start);
                        }
                    }
                }
                break;

            case OSPF::Interface::Broadcast:
                if (intf->getRouterPriority() == 0) {
                    changeState(intf, new OSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    changeState(intf, new OSPF::InterfaceStateWaiting, this);
                    messageHandler->StartTimer(intf->getWaitTimer(), intf->getRouterDeadInterval());
                }
                break;

            default:
                break;
        }
    }
    if (event == OSPF::Interface::LoopIndication) {
        intf->Reset();
        changeState(intf, new OSPF::InterfaceStateLoopback, this);
    }
}

