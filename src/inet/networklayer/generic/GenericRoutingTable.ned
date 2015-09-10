//
// Copyright (C) 2012 Opensim Ltd.
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

package inet.networklayer.generic;

import inet.networklayer.contract.IRoutingTable;

//
// This module stores generic routes used by the generic network protocol.
//
simple GenericRoutingTable like IRoutingTable
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/table");
        string addressType @enum("mac","modulepath","moduleid") = default("modulepath");
        string routerId = default("auto"); // for routers, the router id using address dotted
                          // notation; specify "auto" to select the highest
                          // interface address; should be left empty ("") for hosts
        bool forwarding = default(true);  // turns IP forwarding on/off
        bool multicastForwarding = default(false); // turns multicast forwarding on/off
        @signal[NF_ROUTE_ADDED](type=GenericRoute);
        @signal[NF_ROUTE_DELETED](type=GenericRoute);
        @signal[NF_ROUTE_CHANGED](type=GenericRoute);
}
