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
    "MACRelayUnit",
    "EtherMAC2";


//
// Model of an Ethernet switch built with EtherMAC2, which means
// that all ports are operating in strictly full-duplex mode.
// Use EtherSwitch if you need half-duplex operation on some ports.
//
// This model does not contain the spanning tree algorithm.
//
module EtherSwitch2
    parameters:
        relayUnitType: string; // type of the MACRelayUnit; currently possible
                               // values are MACRelayUnitNP and MACRelayUnitPP
    gates:
        in: in[];
        out: out[];

    submodules:
        relayUnit: relayUnitType like MACRelayUnit;
            gatesizes:
                lowerLayerIn[sizeof(in)],
                lowerLayerOut[sizeof(in)];
            display: "i=greenbox;p=200,50";
        mac: EtherMAC2[sizeof(in)];
            parameters:
                promiscuous = true,
                txQueueLimit = 1000, // increase if needed
                queueModule = "";
            display: "i=block/queue;p=70,150,row;q=queue";
    connections:
        for i=0..sizeof(in)-1 do
            mac[i].upperLayerIn <-- relayUnit.lowerLayerOut[i];
            mac[i].upperLayerOut --> relayUnit.lowerLayerIn[i];
            mac[i].physIn <-- in[i];
            mac[i].physOut --> out[i];
        endfor;
endmodule

