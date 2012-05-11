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

#include "OSPFInterfaceStateNotDesignatedRouter.h"

#include "MessageHandler.h"
#include "OSPFArea.h"
#include "OSPFInterfaceStateDown.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFRouter.h"


void OSPF::InterfaceStateNotDesignatedRouter::processEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::NEIGHBOR_CHANGE) {
        calculateDesignatedRouter(intf);
    }
    if (event == OSPF::Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new OSPF::InterfaceStateLoopback, this);
    }
    if (event == OSPF::Interface::HELLO_TIMER) {
        if (intf->getType() == OSPF::Interface::BROADCAST) {
            intf->sendHelloPacket(IPv4Address::ALL_OSPF_ROUTERS_MCAST);
        } else {    // OSPF::Interface::NBMA
            if (intf->getRouterPriority() > 0) {
                unsigned long neighborCount = intf->getNeighborCount();
                for (unsigned long i = 0; i < neighborCount; i++) {
                    const OSPF::Neighbor* neighbor = intf->getNeighbor(i);
                    if (neighbor->getPriority() > 0) {
                        intf->sendHelloPacket(neighbor->getAddress());
                    }
                }
            } else {
                intf->sendHelloPacket(intf->getDesignatedRouter().ipInterfaceAddress);
                intf->sendHelloPacket(intf->getBackupDesignatedRouter().ipInterfaceAddress);
            }
        }
        intf->getArea()->getRouter()->getMessageHandler()->startTimer(intf->getHelloTimer(), intf->getHelloInterval());
    }
    if (event == OSPF::Interface::ACKNOWLEDGEMENT_TIMER) {
        intf->sendDelayedAcknowledgements();
    }
}

