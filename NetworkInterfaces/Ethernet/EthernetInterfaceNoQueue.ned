//
// Copyright (C) 2004 Andras Varga
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
    "EtherMAC",
    "EtherEncap";

//
// Ethernet network interface. Assumes inifinite queue.
//
// @see EthernetInterface
//
module EthernetInterfaceNoQueue
    gates:
        in: physIn, netwIn;
        out: physOut, netwOut;
    submodules:
        mac: EtherMAC;
            parameters: 
                queueModule = "",
                txQueueLimit = 100000; // prevents overflow in bogus simulations
            display: "i=block/rxtx;p=100,160;q=txQueue";
        encap: EtherEncap;
            display: "i=block/square;p=100,80";
    connections:
        netwIn --> encap.upperLayerIn;
        netwOut <-- encap.upperLayerOut;
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.physIn <-- physIn;
        mac.physOut --> physOut;
endmodule

