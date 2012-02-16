//
// Copyright (C) 2011 Kyeong Soo (Joseph) Kim
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


package inet.nodes.access;

import inet.linklayer.ethernet.EtherMAC2withOutputQueue;
import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.vlan.VLANTagger;


//
// Abstract model of a next-generation network (NGN) access modem
// (e.g., cable modem, xDSL modem, TDM-PON ONU) based on a
// VLAN-capable Ethernet switch with external output queueing,
// VLAN tagger, frame classifier, and traffic control modules
// (i.e., EtherSwitch2WithOutputQueueAndVLAN).
//
// <b>VLAN ID Assignment</b>
//
// VLAN ID is automatically assigned based on the module index.
//
// @see MACRelayUnitNPWithVLAN, VLANTagger, DropTailTBFQueue, VLANClassifier
//
module NgnAccessModem
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        int pvid = default(10);	// port VLAN ID
        string relayUnitType = default("MACRelayUnitNPWithVLAN"); // type of the MACRelayUnit; currently
			   				   									  // only MACRelayUnitNPWithVLAN is possible
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
        tagger[sizeof(ethg)]: VLANTagger {
            parameters:
                @display("p=94,99");
                tagged = index == sizeof(ethg)-1 ? true : false;
                minVid = pvid;
                maxVid = pvid;
        }
        mac[sizeof(ethg)]: EtherMAC2withOutputQueue {
            parameters:
                @display("p=94,184,row;q=queue");
                promiscuous = true;
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            relayUnit.lowerLayerOut[i] --> tagger[i].relayg$i;
            relayUnit.lowerLayerIn[i] <-- tagger[i].relayg$o;
            tagger[i].macg$o --> mac[i].upperLayerIn;
            tagger[i].macg$i <-- mac[i].upperLayerOut;
            mac[i].phys <--> ethg[i];
        }
}