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

package inet.networklayer.generic;

//
// TODO
//
// @author Andras Varga
//
simple GenericNetworkProtocol
{
    parameters:
        string routingTableModule = default("routingTable");
        double procDelay @unit("s") = default(0s);
        int hopLimit = default(32);
        @display("i=block/routing");
    gates:
        input transportIn[] @labels(GenericNetworkProtocolControlInfo/down,TCPSegment,UDPPacket);
        output transportOut[] @labels(GenericNetworkProtocolControlInfo/up,TCPSegment,UDPPacket);
        input queueIn[] @labels(GenericDatagram);
        output queueOut @labels(GenericDatagram);
}
