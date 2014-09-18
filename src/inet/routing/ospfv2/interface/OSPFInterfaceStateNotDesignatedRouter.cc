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

#include "inet/routing/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.h"

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/interface/OSPFInterfaceStateDown.h"
#include "inet/routing/ospfv2/interface/OSPFInterfaceStateLoopback.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

void InterfaceStateNotDesignatedRouter::processEvent(Interface *intf, Interface::InterfaceEventType event)
{
    if (event == Interface::NEIGHBOR_CHANGE) {
        calculateDesignatedRouter(intf);
    }
    if (event == Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    if (event == Interface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new InterfaceStateLoopback, this);
    }
    if (event == Interface::HELLO_TIMER) {
        if (intf->getType() == Interface::BROADCAST) {
            intf->sendHelloPacket(IPv4Address::ALL_OSPF_ROUTERS_MCAST);
        }
        else {    // Interface::NBMA
            if (intf->getRouterPriority() > 0) {
                unsigned long neighborCount = intf->getNeighborCount();
                for (unsigned long i = 0; i < neighborCount; i++) {
                    const Neighbor *neighbor = intf->getNeighbor(i);
                    if (neighbor->getPriority() > 0) {
                        intf->sendHelloPacket(neighbor->getAddress());
                    }
                }
            }
            else {
                intf->sendHelloPacket(intf->getDesignatedRouter().ipInterfaceAddress);
                intf->sendHelloPacket(intf->getBackupDesignatedRouter().ipInterfaceAddress);
            }
        }
        intf->getArea()->getRouter()->getMessageHandler()->startTimer(intf->getHelloTimer(), intf->getHelloInterval());
    }
    if (event == Interface::ACKNOWLEDGEMENT_TIMER) {
        intf->sendDelayedAcknowledgements();
    }
}

} // namespace ospf

} // namespace inet

