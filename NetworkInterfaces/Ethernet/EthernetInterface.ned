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
    "ARP",
    "EtherMAC",
    "EtherLLC";

//
// Ethernet network interface. Corresponds to the prototype NetworkInterface.
//
module EthernetInterface
    parameters:
        proxyARP: bool;  // sets proxy ARP mode
    gates:
        in: physIn, netwIn;
        out: physOut, netwOut;
    submodules:
        arp: ARP;
            parameters:
                proxyARP = proxyARP;
            display: "p=100,80;i=block/layer;q=pendingQueue";
        encap: EtherEncap;
            gatesizes:
                upperLayerIn[1],
                upperLayerOut[1];
            display: "i=block/fork;p=100,160";
        mac: EtherMAC;
            display: "i=block/queue;p=100,240;q=queue";
    connections:
        netwOut <-- arp.hlOut;
        netwIn --> arp.hlIn;
        arp.hwOut --> encap.upperLayerIn;
        arp.hwIn <-- encap.upperLayerOut;
        encap.lowerLayerOut --> mac.upperLayerIn;
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.physicalIn <-- physIn;
        mac.physicalOut --> physOut;
endmodule

