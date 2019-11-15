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
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {
namespace ospfv2 {

void InterfaceStateNotDesignatedRouter::processEvent(Ospfv2Interface *intf, Ospfv2Interface::Ospfv2InterfaceEventType event)
{
    if (event == Ospfv2Interface::NEIGHBOR_CHANGE) {
        calculateDesignatedRouter(intf);
    }
    else if (event == Ospfv2Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    else if (event == Ospfv2Interface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new InterfaceStateLoopback, this);
    }
    else if (event == Ospfv2Interface::HELLO_TIMER) {
        if (intf->getType() == Ospfv2Interface::BROADCAST) {
            intf->sendHelloPacket(Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
        }
        else {    // Ospfv2Interface::NBMA
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
    else if (event == Ospfv2Interface::ACKNOWLEDGEMENT_TIMER) {
        intf->sendDelayedAcknowledgements();
    }
}

} // namespace ospfv2
} // namespace inet

