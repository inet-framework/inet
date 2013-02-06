//
// Copyright (C) 2004 Andras Varga
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
import inet.networklayer.generic.GenericRoutingTable;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.ipv6.IPv6RoutingTable;

//
// TODO
//
module MultiRoutingTable like IRoutingTable
{
    parameters:
        @display("i=block/table");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableGeneric = default(true);

    submodules:
        // TODO: simplify names to ipv4, ipv6, generic once looking up modules support paths
        ipv4RoutingTable: IPv4RoutingTable if enableIPv4 {
            @display("p=100,50;is=s");
        }
        ipv6RoutingTable: IPv6RoutingTable if enableIPv6 {
            @display("p=100,100;is=s");
        }
        genericRoutingTable: GenericRoutingTable if enableGeneric {
            @display("p=100,150;is=s");
        }
}
