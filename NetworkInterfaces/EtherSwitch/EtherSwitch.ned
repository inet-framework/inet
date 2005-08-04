//
// Copyright (C) 2003 CTIE, Monash University
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
    "EtherMAC";


//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
//
// This model does not contain the spanning tree algorithm.
//
module EtherSwitch
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
        mac: EtherMAC[sizeof(in)];
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

