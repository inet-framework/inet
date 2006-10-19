//
// Copyright (C) 2006 Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


import
    "EtherMAC2",
    "EtherEncap",
    "OutputQueue";

//
// Ethernet network interface, which supports full-duplex operation only.
// Corresponds to the prototype NetworkInterface.
// Complements EtherMAC2 and EtherEncap with an output queue for QoS and
// RED support.
//
// @see EthernetInterface, EthernetInterfaceNoQueue
//
module EthernetInterface2
    parameters:
        queueType: string;
    gates:
        in: physIn;
        in: netwIn;
        out: physOut;
        out: netwOut;
    submodules:
        queue: queueType like OutputQueue;
            display: "i=block/queue;p=92,71;q=l2queue";
        mac: EtherMAC2;
            parameters:
                queueModule = "queue",
                txQueueLimit = 1; // queue sends one packet at a time
            display: "i=block/rxtx;p=116,231";
        encap: EtherEncap;
            display: "i=block/square;p=116,151";
    connections:
        netwIn --> queue.in;
        queue.out --> encap.upperLayerIn;
        netwOut <-- encap.upperLayerOut display "m=n";
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.physIn <-- physIn;
        mac.physOut --> physOut;
endmodule

