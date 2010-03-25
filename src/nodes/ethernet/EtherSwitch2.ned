//
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




package inet.nodes.ethernet;

import inet.linklayer.ethernet.EtherMAC2;
import inet.linklayer.etherswitch.MACRelayUnit;


//
// Model of an Ethernet switch built with EtherMAC2, which means
// that all ports are operating in strictly full-duplex mode.
// Use EtherSwitch if you need half-duplex operation on some ports.
//
// This model does not contain the spanning tree algorithm.
//
module EtherSwitch2
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        string relayUnitType = default("MACRelayUnitNP"); // type of the MACRelayUnit; currently possible
                                                          // values are MACRelayUnitNP and MACRelayUnitPP
    gates:
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        relayUnit: <relayUnitType> like MACRelayUnit {
            parameters:
                @display("p=200,50");
            gates:
                lowerLayerIn[sizeof(ethg)];
                lowerLayerOut[sizeof(ethg)];
        }
        mac[sizeof(ethg)]: EtherMAC2 {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=70,150,row;q=queue");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            mac[i].upperLayerIn <-- relayUnit.lowerLayerOut[i];
            mac[i].upperLayerOut --> relayUnit.lowerLayerIn[i];
            mac[i].phys <--> ethg[i];
        }
}

