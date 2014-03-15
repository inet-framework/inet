//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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

import inet.linklayer.ethernet.EtherMAC2withOutputQueueAndMeter;
import inet.linklayer.etherswitch.MACRelayUnit;
import inet.linklayer.vlan.VLANTagger;


//
// Abstract model of a next-generation network (NGN) access node
// (e.g., CMTS, DSLAM, PON OLT) based on a VLAN-capable Ethernet
// relay unit with external output queueing, VLAN tagger, frame
// classifier, packet QoS meter, and traffic control modules
// (i.e., EtherSwitch2WithOutputQueueAndVLAN).
//
// <b>Traffic Control</b>
//
// To model a switching node at the headend of shared access networks
// like an optical line termination (OLT) in passive optical networks
// (PONs) or a cable modem termination system (CMTS) in cable
// networks, classification of frames (e.g., based on MAC addresses or
// the PCP field of IEEE 802.1Q Tag) and traffic control per class are
// required. Currently, traffic control based on the well-known token
// bucket (TB) algorithm is implemented to control both mean
// rate/maximum burst and peak rate/MTU through dual TBFs.
//
// <b>VLAN ID Assignment</b>
//
// VLAN IDs are automatically assigned based on the number of subscribers.
//
// @see MACRelayUnitNPWithVLAN, VLANTagger, DropTailTBFQueue, VLANClassifier
//
module NgnAccessNode2
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        int numSubscribers = default(1);	// number of subscribers (e.g., ONUs)
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
                tagged = index == 0 ? true : false;
                dynamicTagging = index == 0 ? false : true;
                minVid = 10;
                maxVid = 10+(numSubscribers-1);       
        }
        mac[sizeof(ethg)]: EtherMAC2withOutputQueueAndMeter {
            parameters:
                @display("p=94,184,row;q=queue");
                promiscuous = true;
//                queueType = index == 0 ? "DRRVLANQueue" : "DropTailQueue";
//                queueType = index == 0 ? "CSFQVLANTokenBucketQueue" : "DropTailQueue";
                numFlows = index == 0 ? numSubscribers : 0;
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