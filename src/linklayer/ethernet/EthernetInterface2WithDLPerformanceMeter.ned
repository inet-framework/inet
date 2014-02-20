//
// Copyright (C) 2014 Kyeong Soo (Joseph) Kim
// Copyright (C) 2006 Levente Meszaros
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

import inet.linklayer.INetworkInterface;
import inet.networklayer.queue.OutputQueue;
import inet.util.PerformanceMeter;


//
// Ethernet network interface which supports full-duplex operation only.
// Complements EtherMAC2 and EtherEncap with an output queue for QoS and
// RED support and download end-to-end frame delay measurement
//
// @see EthernetInterface, EthernetInterfaceNoQueue
//
module EthernetInterface2WithDLPerformanceMeter like INetworkInterface
{
    parameters:
        string queueType = default("DropTailQueue");
        @display("i=block/ifcard;bgb=245,315");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(EtherFrame);
    submodules:
        queue: <queueType> like OutputQueue {
            parameters:
                @display("p=53,48;q=l2queue");
        }
        mac: EtherMAC2 {
            parameters:
                queueModule = "queue";
                txQueueLimit = 1; // queue sends one packet at a time
                @display("p=116,231");
        }
        encap: EtherEncap {
            parameters:
                @display("p=116,103");
        }
        performanceMeter: PerformanceMeter {
            parameters:
                @display("p=187,157");
        }
    connections:
        netwIn --> queue.in;
        queue.out --> encap.upperLayerIn;
        //        netwOut <-- thruputMeter.out;
        //        thruputMeter.in <-- { @display("m=n"); } <-- encap.upperLayerOut;
        netwOut <-- encap.upperLayerOut;
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- performanceMeter.out;
        performanceMeter.in <-- { @display("m=n"); } <-- mac.upperLayerOut;
        mac.phys <--> phys;
}

