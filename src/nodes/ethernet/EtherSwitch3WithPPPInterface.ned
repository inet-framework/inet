//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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

import inet.linklayer.ethernet.EtherMAC3;
import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.ppp.PPPInterface;


//
// Model of an Ethernet switch built with EtherMAC3 and PPP
// interfaces for WAN connections. All ports are operating in
// strictly full-duplex mode and that txQueue now handles frame
// loss in a usual way.
// Use EtherSwitch if you need half-duplex operation on some ports.
//
// This model does not contain the spanning tree algorithm.
//
module EtherSwitch3WithPPPInterface
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch;bgb=300,200");
        string relayUnitType = default("MACRelayUnitNP"); // type of the MACRelayUnit; currently possible
                                                          // values are MACRelayUnitNP and MACRelayUnitPP
    gates:
        inout ethg[] @labels(EtherFrame-conn);
        inout pppg[];	// PPP interface for WAN connections)
    submodules:
        relayUnit: <relayUnitType> like MACRelayUnit {
            parameters:
                @display("p=150,40");
            gates:
                lowerLayerIn[sizeof(ethg)+sizeof(pppg)];
                lowerLayerOut[sizeof(ethg)+sizeof(pppg)];
        }
        mac[sizeof(ethg)]: EtherMAC3 {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=75,135,row;q=queue");
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=220,135,row,90;q=txQueue");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            mac[i].upperLayerIn <-- relayUnit.lowerLayerOut++;
            mac[i].upperLayerOut --> relayUnit.lowerLayerIn++;
            mac[i].phys <--> ethg[i];
        }
        for i=0..sizeof(pppg)-1 {
            ppp[i].netwIn <-- relayUnit.lowerLayerOut++;
            ppp[i].netwOut --> relayUnit.lowerLayerIn++;
            ppp[i].phys <--> pppg[i];
        }
}

