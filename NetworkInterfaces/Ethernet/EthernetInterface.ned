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
        procdelay : numeric const;
    gates:
        in: physicalIn, ipOutputQueueIn;
        out: physicalOut, ipOutputQueueOut, ipInputQueueOut;
    submodules:
        arp: ARP;
            display: "i=proc2;p=155,60";
        encap: EtherEncap;
            gatesizes:
                UpperLayer_in[1],
                UpperLayer_out[1];
            display: "i=fork;p=155,120";
        mac: EtherMAC;
            display: "i=queue;p=155,180";
    connections:
        ipInputQueueOut <-- arp.hlOut;
        ipOutputQueueIn --> arp.hlIn;
        arp.hwOut --> encap.UpperLayer_in;
        arp.hwIn <-- encap.UpperLayer_out;
        encap.LowerLayer_out --> mac.UpperLayer_in;
        encap.LowerLayer_in <-- mac.UpperLayer_out;
        mac.LowerLayer_in <-- physicalIn;
        mac.LowerLayer_out --> physicalOut;
endmodule


