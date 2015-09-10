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
//

package inet.networklayer.generic;

//
// This module is a simplified generic network protocol that routes
// generic datagrams using different kind of network addresses. 
//
// @author Andras Varga
//
simple GenericNetworkProtocol
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string arpModule;
        double procDelay @unit("s") = default(0s);
        int hopLimit = default(32);
        @display("i=block/routing");
    gates:
        input transportIn[] @labels(GenericNetworkProtocolControlInfo/down,TCPSegment,UDPPacket);
        output transportOut[] @labels(GenericNetworkProtocolControlInfo/up,TCPSegment,UDPPacket);
        input arpIn;
        output arpOut;
        input queueIn[] @labels(GenericDatagram);
        output queueOut[] @labels(GenericDatagram);
}
