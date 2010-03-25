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



package inet.linklayer.ethernet;

//
// Ethernet network interface. Assumes inifinite queue.
//
// @see EthernetInterface
//
module EthernetInterfaceNoQueue
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(EtherFrame);
    submodules:
        mac: EtherMAC {
            parameters:
                queueModule = "";
                txQueueLimit = 100000; // prevents overflow in bogus simulations
                @display("p=100,160;q=txQueue");
        }
        encap: EtherEncap {
            parameters:
                @display("p=100,80");
        }
    connections:
        netwIn --> encap.upperLayerIn;
        netwOut <-- encap.upperLayerOut;
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.phys <--> phys;
}

