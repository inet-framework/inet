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

#include "inet/routing/ospfv2/interface/OspfInterfaceStateDown.h"
#include "inet/routing/ospfv2/interface/OspfInterfaceStateLoopback.h"
#include "inet/routing/ospfv2/interface/OspfInterfaceStatePointToPoint.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {
namespace ospf {

void InterfaceStatePointToPoint::processEvent(OspfInterface *intf, OspfInterface::OspfInterfaceEventType event)
{
    if (event == OspfInterface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    else if (event == OspfInterface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new InterfaceStateLoopback, this);
    }
    else if (event == OspfInterface::HELLO_TIMER) {
        if (intf->getType() == OspfInterface::VIRTUAL) {
            if (intf->getNeighborCount() > 0) {
                intf->sendHelloPacket(intf->getNeighbor(0)->getAddress(), VIRTUAL_LINK_TTL);
            }
        }
        else {
            intf->sendHelloPacket(Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
        }
        intf->getArea()->getRouter()->getMessageHandler()->startTimer(intf->getHelloTimer(), intf->getHelloInterval());
    }
    else if (event == OspfInterface::ACKNOWLEDGEMENT_TIMER) {
        intf->sendDelayedAcknowledgements();
    }
}

} // namespace ospf
} // namespace inet

