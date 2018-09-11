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

package inet.networklayer.nexthop;

//
// This module is a simplified next hop forwarding that routes
// datagrams using different kind of network addresses.
//
// @author Andras Varga
//
simple NextHopForwarding
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string arpModule;
        double procDelay @unit(s) = default(0s);
        int hopLimit = default(32);
        int headerLength @unit(B) = default(10B);
        @display("i=block/routing");
        @signal[packetDropped](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
    gates:
        input transportIn @labels(TcpHeader,UdpHeader);
        output transportOut @labels(TcpHeader,UdpHeader);
        input queueIn @labels(NextHopDatagram);
        output queueOut @labels(NextHopDatagram);
}

