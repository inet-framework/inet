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


#include "OSPFInterfaceStateLoopback.h"

#include "OSPFInterfaceStateDown.h"


void OSPF::InterfaceStateLoopback::processEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::UNLOOP_INDICATION) {
        changeState(intf, new OSPF::InterfaceStateDown, this);
    }
}

