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
// Abstract model of a next-generation network (NGN) splitter
// (e.g., shared medium) based on a VLAN-capable Ethernet relay unit
// with external output queueing, VLAN tagger, frame classifier, and
// traffic control modules (i.e., EtherSwitch2WithOutputQueueAndVLAN).
//
// <b>VLAN ID Assignment</b>
//
// VLAN IDs are automatically assigned based on the number of downstream ports.
//
// @see MACRelayUnitNPWithVLAN, VLANTagger, DropTailTBFQueue, VLANClassifier
//
module NgnAccessSplitter
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
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
                tagged = true;
                minVid = index < sizeof(ethg)-1 ? 10+index : 10;
                maxVid = index < sizeof(ethg)-1 ? this.minVid : 10+(sizeof(ethg)-2);       
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