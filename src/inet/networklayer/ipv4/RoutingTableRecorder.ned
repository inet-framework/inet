//
// Copyright (C) 2012 Andras Varga
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


package inet.networklayer.ipv4;

//
// Records changes in the routing tables (~IPv4RoutingTable) and interface tables
// (~InterfaceTable) of all hosts and routers. The filename has to be specified
// in the routinglog-file configuration option that this module registers.
//
simple RoutingTableRecorder
{
    parameters:
        bool enabled = default(true);
        @display("i=block/control_s");
}

