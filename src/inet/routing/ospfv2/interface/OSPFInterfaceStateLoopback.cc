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

#include "inet/routing/ospfv2/interface/OSPFInterfaceStateLoopback.h"

#include "inet/routing/ospfv2/interface/OSPFInterfaceStateDown.h"

namespace inet {

namespace ospf {

void InterfaceStateLoopback::processEvent(Interface *intf, Interface::InterfaceEventType event)
{
    if (event == Interface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    if (event == Interface::UNLOOP_INDICATION) {
        changeState(intf, new InterfaceStateDown, this);
    }
}

} // namespace ospf

} // namespace inet

