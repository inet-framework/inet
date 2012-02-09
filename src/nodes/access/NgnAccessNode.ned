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
// Abstract model of a next-generation network (NGN) access node
// (e.g., CMTS, DSLAM, PON OLT) based on a VLAN-capable Ethernet
// relay unit with external output queueing, VLAN tagger, frame
// classifier, and traffic control modules.
//
// <b>Traffic Control</b>
//
// To model a switching node at the headend of shared access networks
// like an optical line termination (OLT) in passive optical networks
// (PONs) or a cable modem termination system (CMTS) in cable
// networks, classification of frames (e.g., based on MAC addresses or
// the PCP field of IEEE 802.1Q Tag) and traffic control per class are
// required. Currently, traffic shaping based on the well-known token
// bucket filter (TBF) is implemented to control both mean
// rate/maximum burst and peak rate/MTU through dual TBFs.
//
// @see MACRelayUnitNPWithVLAN, VLANTagger, DropTailTBFQueue, VLANClassifier
//
module NgnAccessNode
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch;bgb=242,239");
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
        }
        mac[sizeof(ethg)]: EtherMAC2withOutputQueue {
            parameters:
                promiscuous = true;
                @display("p=94,184,row;q=queue");
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
