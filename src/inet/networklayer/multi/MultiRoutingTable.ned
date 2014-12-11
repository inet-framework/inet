//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

package inet.networklayer.multi;

import inet.networklayer.contract.IRoutingTable;

//
// This module supports multiple different routing tables simultaneously.
// The actual routes are stored in the submodule routing tables and they
// are accessed directly by modules.
//
module MultiRoutingTable like IRoutingTable
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/table");
        bool enableIPv4 = default(haveClass("inet::IPv4RoutingTable"));
        bool enableIPv6 = default(haveClass("inet::IPv6RoutingTable"));
        bool enableGeneric = default(haveClass("inet::GenericRoutingTable"));
        bool forwarding;
        bool multicastForwarding;

    submodules:
        ipv4: <"IPv4RoutingTable"> like IRoutingTable if enableIPv4 {
            @display("p=50,50;is=s");
            forwarding = forwarding;
            multicastForwarding = multicastForwarding;
        }
        ipv6: <"IPv6RoutingTable"> like IRoutingTable if enableIPv6 {
            @display("p=50,100;is=s");
            forwarding = forwarding;
            multicastForwarding = multicastForwarding;
        }
        generic: <"GenericRoutingTable"> like IRoutingTable if enableGeneric {
            @display("p=50,150;is=s");
            forwarding = forwarding;
            multicastForwarding = multicastForwarding;
        }
}
