//
// Copyright (C) 2001 Monash University, Australia
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
    "OutputQueue",
    "NetworkInterface";



//
// Network interface, with the actual L2 protocol left as parameter.
//
// Outgoing packets are queued before entering the Link Layer protocol
// (performance modelling).
//
// The Link Layer protocol should be selected with a string parameter
// (it is instantiated via "<tt>like</tt>"). Possible values are... (FIXME)
//
module LinkLayer
    parameters:
        // Link Layer network name
        NWIName : string;
    gates:
        in: physIn; // from physical layer
        out: physOut; // to physical layer
        in: outputQueueIn; // from upper layer (L3)
        out: inputQueueOut; // to upper layer (L3)

    submodules:
        outputQueue: OutputQueue;
            display: "b=32,30;p=138,63;i=queue";
        networkInterface: NWIName like NetworkInterface;
            display: "b=32,30;p=87,125;i=bwxconn";
    connections nocheck:

        // IP layer to L2 Output Queue
        outputQueueIn --> outputQueue.fromIP;

        // L2 Output Queue to Network interface
        networkInterface.ipOutputQueueIn <-- outputQueue.toNW;
        networkInterface.ipOutputQueueOut --> outputQueue.fromNW;

        // physical connection
        networkInterface.physicalOut --> physOut;
        networkInterface.physicalIn <-- physIn;

        networkInterface.ipInputQueueOut --> inputQueueOut;

endmodule

