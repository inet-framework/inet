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

package inet.emulation.networklayer.ipv4;

import inet.networklayer.ipv4.IIpv4;

module ExtLowerIpv4 like IIpv4
{
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
    submodules:
        encap: Ipv4Encap {
            parameters:
	            @display("p=100,50");
        }
        socket: ExtIpv4Socket {
            parameters:
	            @display("p=100,150");
        }
    connections allowunconnected:
        transportIn --> encap.upperLayerIn;
        transportOut <-- encap.upperLayerOut;
        encap.lowerLayerOut --> socket.upperLayerIn;
        encap.lowerLayerIn <-- socket.upperLayerOut;
}
