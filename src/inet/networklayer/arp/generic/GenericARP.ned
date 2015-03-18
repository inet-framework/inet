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

package inet.networklayer.arp.generic;

import inet.networklayer.contract.IARP;


//
// TODO
//
simple GenericARP like IARP
{
    parameters:
        @display("i=block/layer_s");
        string interfaceTableModule;   // The path to the InterfaceTable module
    gates:
        input netwIn @labels(IGenericDatagram); // incoming ARP requests and replies
        output netwOut @labels(IGenericDatagram);  // outgoing ARP requests/replies, and datagrams with resolved next hop
}
