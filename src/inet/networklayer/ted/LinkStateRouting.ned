//
// Copyright (C) 2005 Vojtech Janota, Andras Varga
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.networklayer.ted;

//
// This module implements a very minimalistic link state routing protocol.
// Apart from the basic topology information, the current link usage
// is distributed to all participants in the network (by means of flooding).
//
// Collaborations
//
// The link state database is kept in the ~Ted module.
//
simple LinkStateRouting
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string tedModule;
        string peers;
        @display("i=block/network2");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}

