//
// Copyright (C) OpenSim Ltd.
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

package inet.emulation.linklayer.ethernet;

import inet.emulation.common.ExtInterface;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.IEtherEncap;

//
// This module provides an Ethernet network interface suitable for emulation.
// The lower part of the network interface is realized in the real world using
// a real ethernet socket of the host computer which is running the simulation.
//
module ExtLowerEthernetInterface extends ExtInterface like IEthernetInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        inout phys @loose;
    submodules:
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=100,100");
        }
        socket: ExtEthernetSocket {
            parameters:
                @display("p=100,300");
        }
    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if exists(encap);
        upperLayerOut <-- { @display("m=n"); } <-- encap.upperLayerOut if exists(encap);
        encap.lowerLayerOut --> socket.upperLayerIn if exists(encap);
        encap.lowerLayerIn <-- socket.upperLayerOut if exists(encap);

        upperLayerIn --> { @display("m=n"); } --> socket.upperLayerIn if !exists(encap);
        upperLayerOut <-- { @display("m=n"); } <-- socket.upperLayerOut if !exists(encap);
}
