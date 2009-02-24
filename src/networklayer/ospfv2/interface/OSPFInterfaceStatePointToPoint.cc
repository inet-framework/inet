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

#include "OSPFInterfaceStatePointToPoint.h"
#include "OSPFInterfaceStateDown.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include "MessageHandler.h"

void OSPF::InterfaceStatePointToPoint::ProcessEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::InterfaceDown) {
        intf->Reset();
        ChangeState(intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::LoopIndication) {
        intf->Reset();
        ChangeState(intf, new OSPF::InterfaceStateLoopback, this);
    }
    if (event == OSPF::Interface::HelloTimer) {
        if (intf->GetType() == OSPF::Interface::Virtual) {
            if (intf->GetNeighborCount() > 0) {
                intf->SendHelloPacket(intf->GetNeighbor(0)->GetAddress(), VIRTUAL_LINK_TTL);
            }
        } else {
            intf->SendHelloPacket(OSPF::AllSPFRouters);
        }
        intf->GetArea()->GetRouter()->GetMessageHandler()->StartTimer(intf->GetHelloTimer(), intf->GetHelloInterval());
    }
    if (event == OSPF::Interface::AcknowledgementTimer) {
        intf->SendDelayedAcknowledgements();
    }
}

