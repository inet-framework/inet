//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.configurator;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module configures the forwarding database (MAC address table) of all
// network nodes in the network based on the automatically discovered network
// topology. The configurator uses the shortest path algorithm to determine the
// outgoing network interface for all network nodes and all destination network
// interfaces. The effect of this configuration is that the network can use the
// ~GlobalArp module and completely eliminate the ARP protocol messages.
//
// @see ~MacAddressTable
//
simple MacAddressTableConfigurator like INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}

