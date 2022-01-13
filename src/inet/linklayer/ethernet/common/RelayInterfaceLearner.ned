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

package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module extracts the source MAC address of the packet passing through and
// stores the mapping from this MAC address to the incoming network interface in
// the MAC address table (forwarding information database).
//
simple RelayInterfaceLearner extends PacketFlowBase like IPacketFlow
{
    parameters:
        string macTableModule; // relative module path of MAC table
        string interfaceTableModule; // relative module path of interface table
        @class(RelayInterfaceLearner);
        @display("i=block/blackboard");
}

